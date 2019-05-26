/*
 * Tasks.c
 *
 *  Created on: May 26, 2019
 *      Author: oleg
 */

#include <string.h>

#include "application/Tasks.h"
#include "application/ADCController.h"
#include "application/CommandStructs.h"
#include "application/CommandParser.h"
#include "application/ConstantsCommon.h"
#include "application/Protocol.h"
#include "application/Settings.h"
#include "application/Config.h"
#include "application/Diagnostic.h"
#include "application/HetConstants.h"
#include "application/Wheels.h"
#include "application/HetPinsController.h"
#include "application/FEEController.h"

#include "RtosWrapper/Rtos.h"
#include "RtosWrapper/RtosQueue.h"


#define DUMMY_BREAK if (0) break


static Queue commandsQueue;
static Queue memoryCommandsQueue;
static Queue wheelsCommandsQueues[WHEELS_COUNT];
static Queue adcValuesQueues[ADC_FIFO_SIZE];    // adc values have specific order, see ADCController.h(.c)
#ifndef SIMPLE_WHEEL_LOGIC
static Queue adcAverageQueue;
#endif


static LevelValues cachedLevels[LEVELS_COUNT];
static Settings cachedSettings;
static Diagnostic diagnostic;
static LevelValues* pCurrentTargetLevels = NULL;

/*=================================================*/
#pragma SWI_ALIAS(swiSwitchToMode, 1)

// Mode = 0x10 for user and 0x1F for system mode
extern void swiSwitchToMode ( uint32 mode );

/*=================================================*/
inline void upWheel(WheelPinsStruct wheelPins)
{
    openPin(wheelPins.upPin);
    closePin(wheelPins.downPin);
}

inline void downWheel(WheelPinsStruct wheelPins)
{
    closePin(wheelPins.upPin);
    openPin(wheelPins.downPin);
}

inline void stopWheel(WheelPinsStruct wheelPins)
{
    closePin(wheelPins.upPin);
    closePin(wheelPins.downPin);
}


inline bool getBatteryVoltage(long* const retVoltage)
{
    uint16 adcValue = 0;

    if (readFromQueueWithTimeout(&adcValuesQueues[BATTERY_IDX], &adcValue, MS_TO_TICKS(1000)))
    {
        *retVoltage = (portLONG)(adcValue *
                                (5.0 / 255.0) *     // convert to Volts, ADC 8 bit
                                (147.0 / 47.0) *    // devider 4.7k/14.7k
                                1000);              // milivolts

        return true;
    }

    return false;
}

inline bool getWheelLevelValue(const portSHORT wheelNumber, uint16 * const retLevel)
{
    return readFromQueueWithTimeout(&adcValuesQueues[wheelNumber], retLevel, MS_TO_TICKS(2000));
}


bool getCurrentWheelsLevelsValues(LevelValues* const retLevels)
{
    portSHORT i = 0;
    for (; i < WHEELS_COUNT; ++i)
    {
        if (!getWheelLevelValue(i, &(retLevels->wheels[i])))
        {
            printError(QueueReadTimeoutErrorCode, "Timeout at wheel level value reading from the queue");
            return false;
        }
    }

    return true;
}


bool setCachedWheelLevel(uint8_t levelNumber, LevelValues values)
{
    bool rv = false;

    if (levelNumber < LEVELS_COUNT) {
        cachedLevels[levelNumber] = values;
        rv = true;
    }

    return rv;
}


const LevelValues* getCachedWheelLevels(void)
{
    return cachedLevels;
}


Settings* getSettings(void)
{
    return &cachedSettings;
}


bool getCompressorPressure(AdcValue_t* const retLevel)
{
    if (!readFromQueueWithTimeout(&adcValuesQueues[COMPRESSOR_IDX], retLevel, 0))
    {
        printError(QueueReadTimeoutErrorCode, "Timeout at compressor value reading from the queue");
        return false;
    }
    return true;
}


// TODO: move to separate file as strategy
void executeWheelLogic(WheelStatusStruct* wheelStatus)
{
    /*
     * Check wheel level
     *
     * Here we should check that we are not exceeded a needed level.
     * If it happens then stop wheel and wait for a new command.
     * If not - continue cycle execution.
     */

    AdcValue_t levelValue = 0;
    if (!readFromQueueWithTimeout(&adcValuesQueues[wheelStatus->wheelNumber], &levelValue, 0))
    {
        printError(QueueReadTimeoutErrorCode, "Timeout at level value reading from the queue");
        wheelStatus->isWorking = false;
        return;
    }

    boolean allowWheelUp = levelValue < cachedSettings.levels_values_max.wheels[wheelStatus->wheelNumber];
    boolean allowWheelDown = levelValue > cachedSettings.levels_values_min.wheels[wheelStatus->wheelNumber];

    // auto mode checks
    if (wheelStatus->levelLimitValue > 0)
    {
#ifndef SIMPLE_WHEEL_LOGIC
        AdcValue_t average_delta = 0;
        if (!readFromQueueWithTimeout(&adcAverageQueue, &average_delta, 0))
        {
            printError(QueueReadTimeoutErrorCode, "Timeout at average level value reading from the queue");
            wheelStatus->isWorking = false;
            return;
        }

        boolean shouldRun = (average_delta >= WHEELS_LEVELS_DEVIATION);
        if (!shouldRun)
        {
            wheelStatus->isWorking = false;
        }
        else
#endif
        {
            boolean shouldWheelUp = levelValue < (wheelStatus->levelLimitValue - WHEELS_LEVELS_THRESHOLD);
            boolean shouldWheelDown = levelValue > (wheelStatus->levelLimitValue + WHEELS_LEVELS_THRESHOLD);

            if (shouldWheelUp && allowWheelUp)
            {
                wheelStatus->cmdType = CMD_WHEEL_UP;
            }
            else if (shouldWheelDown && allowWheelDown)
            {
                wheelStatus->cmdType = CMD_WHEEL_DOWN;
            }
            else
            {
                wheelStatus->cmdType = CMD_WHEEL_STOP;
            }
        }
    } // auto mode if (wheelStatus->levelLimitValue > 0)'
    // manual mode checks
    else
    {
        if (wheelStatus->cmdType == CMD_WHEEL_UP && !allowWheelUp)
            wheelStatus->cmdType = CMD_WHEEL_STOP;

        if (wheelStatus->cmdType == CMD_WHEEL_DOWN && !allowWheelDown)
            wheelStatus->cmdType = CMD_WHEEL_STOP;
    }

    /*
     * Check timer
     */
    if (wheelStatus->isWorking)
    {
        volatile portSHORT elapsedTimeSec = (getTickCount() - wheelStatus->startTime) / configTICK_RATE_HZ;
        wheelStatus->isWorking = elapsedTimeSec < wheelStatus->timeoutSec;
    }
}  // executeWheelLogic


void sendToExecuteCommand(Command cmd)
{
    if (cmd.commandType == UNKNOWN_COMMAND)
    {
        printError(UnknownCommandErrorCode, "Unknown command received");
        return;
    }

    if ((cmd.commandType & ENV_COMMAND_TYPE) == ENV_COMMAND_TYPE)
    {
        if (cmd.commandType == CMD_GET_VERSION)
        {
            executeCommand(&cmd);
        }

        if (cmd.commandType == CMD_GET_BATTERY)
        {
            executeCommand(&cmd);
        }

        if (cmd.commandType == CMD_HELP)
        {
            executeCommand(&cmd);
        }

        // TODO: remove after moving pressure value to the diagnostic
        if (cmd.commandType == CMD_GET_COMPRESSOR_PRESSURE)
        {
            executeCommand(&cmd);
        }

        if (cmd.commandType == CMD_SET_COMPRESSOR_MIN_PRESSURE ||
            cmd.commandType == CMD_SET_COMPRESSOR_MAX_PRESSURE ||
            cmd.commandType == CMD_GET_COMPRESSOR_MIN_PRESSURE ||
            cmd.commandType == CMD_GET_COMPRESSOR_MAX_PRESSURE)
        {
            if (!sendToQueueWithTimeout(&memoryCommandsQueue, (void*) &cmd, 0))
            {
                printError(MemoryQueueErrorCode, "Could not add memory command to the queue (it is full).");
            }
        }
    }

    if ((cmd.commandType & WHEEL_COMMAND_TYPE) == WHEEL_COMMAND_TYPE)
    {
        // detect up or down based on current level values.
        if (cmd.commandType == CMD_WHEEL_AUTO)
        {
            printSuccess();

            LevelValues currentLevels;
            if (cmd.argc > 0 && getCurrentWheelsLevelsValues(&currentLevels))
            {
                portSHORT savedLevelNumber = cmd.argv[0] > LEVELS_COUNT ? 0 : cmd.argv[0];
                pCurrentTargetLevels = cachedLevels + savedLevelNumber;

                // translate to new command, where the first argument is a wheel number
                Command newCmd;
                portSHORT i = 0;
                for (; i< WHEELS_COUNT; ++i)
                {
                    if (currentLevels.wheels[i] == pCurrentTargetLevels->wheels[i])
                        continue;

                    newCmd.commandType = (currentLevels.wheels[i] < pCurrentTargetLevels->wheels[i]) ? CMD_WHEEL_UP : CMD_WHEEL_DOWN;
                    newCmd.argv[0] = i;   // not used for 'auto', but level number should be at argv[1] anyway
                    newCmd.argv[1] = savedLevelNumber;  // level number
                    newCmd.argv[2] = WHEEL_TIMER_TIMEOUT_SEC;   // default value for auto mode. For single mode timeout is in execution task.

                    // check the timeout param (in seconds)
                    // and override second argument if it is needed
                    if (cmd.argc > 1)
                    {
                        newCmd.argv[2] = cmd.argv[1];
                    }
                    newCmd.argc = 3;

                    sendToQueueOverride(&wheelsCommandsQueues[i], (void*)&newCmd); // always returns pdTRUE
                }
            }
        }
        // execute any other WHEEL_COMMAND_TYPE command for all wheels if there is no arguments (up, down or stop)
        else if (cmd.argc == 0)
        {
            printSuccess();

            // for all wheels
            portSHORT i = 0;
            for (; i< WHEELS_COUNT; ++i)
            {
                sendToQueueOverride(&wheelsCommandsQueues[i], (void*)&cmd); // always returns pdTRUE
            }
        }
        // execute any other WHEEL_COMMAND_TYPE command for specific wheel if there is at least one parameter (up, down or stop)
        else
        {
            WHEEL_IDX wheelNo = (WHEEL_IDX)cmd.argv[0];
            if (wheelNo < WHEELS_COUNT)
            {
                printSuccess();
                sendToQueueOverride(&wheelsCommandsQueues[wheelNo], (void*)&cmd); // always returns pdTRUE
            }
            else
            {
                printError(WrongWheelSpecifiedErrorCode, "Wrong wheel number specified");
            }
        }
    }

    if ((cmd.commandType & LEVELS_COMMAND_TYPE) == LEVELS_COMMAND_TYPE)
    {
        // add command to the memory task queue: clear, save, print levels.
        // if arguments is empty, then default cell number is 0 (see vMemTask for details).
        if (!sendToQueueWithTimeout(&memoryCommandsQueue, (void*)&cmd, 0))
        {
            printError(MemoryQueueErrorCode, "Could not add memory command to the queue (it is full).");
        }
    }
}


inline void initializeWheelStatus(WheelStatusStruct* wheelStatus, Command* cmd)
{
    wheelStatus->isWorking = true;
    wheelStatus->startTime = getTickCount();
    wheelStatus->cmdType = cmd->commandType;
    wheelStatus->timeoutSec = WHEEL_TIMER_SINGLE_TIMEOUT_SEC;

    // check if there is a level limit value. If not, just up or down a wheel.
    if (cmd->argc >= 2)
    {
        portSHORT savedLevelNumber = cmd->argv[1];
        if (savedLevelNumber < LEVELS_COUNT)
            wheelStatus->levelLimitValue = cachedLevels[savedLevelNumber].wheels[wheelStatus->wheelNumber];
    }

    // check if there is a timeout specified (in seconds)
    if (cmd->argc >= 3)
    {
        portSHORT seconds = cmd->argv[2];
        wheelStatus->timeoutSec = seconds;
    }
}


inline void resetWheelStatus(WheelStatusStruct* status)
{
    status->isWorking = false;
    status->levelLimitValue = 0;
    status->startTime = 0;
    status->cmdType = UNKNOWN_COMMAND;
}


void commandReceivedCallback(uint8_t* receivedCommand, short length)
{
    sendToQueueFromISR(&commandsQueue, receivedCommand);
}


/*=================================================*/
bool tasks_init(void)
{
    memset(&diagnostic, 0, sizeof(Diagnostic));

    createQueue(MAX_COMMANDS_QUEUE_LEN, MAX_COMMAND_LEN, &commandsQueue);

    portSHORT i = 0;
    for (; i< WHEELS_COUNT; ++i)
    {
        createQueue(1, sizeof(Command), wheelsCommandsQueues + i);  // the only command for each wheel
    }

    for (i = 0; i< ADC_FIFO_SIZE; ++i)
    {
        createQueue(1, sizeof(AdcValue_t), adcValuesQueues + i);  // the only value for each wheel
    }

    createQueue(3, sizeof(Command), &memoryCommandsQueue);
#ifndef SIMPLE_WHEEL_LOGIC
    createQueue(1, sizeof(AdcValue_t), &adcAverageQueue);
#endif

    return protocol_init();
}


void vMemTask( void *pvParameters )
{
    swiSwitchToMode(0x1F);

    initializeFEE();

    // update cached levels to actual values
    readLevels((void*)&cachedLevels);
    readSettings((void*)&cachedSettings);

    Command cmd;
    for( ;; )
    {
        boolean result = popFromQueue(&memoryCommandsQueue,  &cmd);
        if (result)
        {
            executeCommand(&cmd);
        }
        else
        {
            printError(MemoryQueueErrorCode, "Could not pop command from the memory queue.");
        }

        DUMMY_BREAK;
    }

    deleteTask();
}


/*
 * ADCUpdaterTask always tries to update ADC values by getADCValues.
 * A task pushes to queue an updated values.
 * Not sure that there is needed some delay. Anyway we should test it.
 * */

void vADCUpdaterTask( void *pvParameters )
{
    AdcDataValues adc_data;
    for( ;; )
    {
        getADCDataValues(&adc_data);

        for (portSHORT i = 0; i < ADC_FIFO_SIZE; ++i)
        {
            sendToQueueOverride(&adcValuesQueues[i], &(adc_data.values[i]));
        }

#ifndef SIMPLE_WHEEL_LOGIC

        /*
         * Calculate average deviation value of levels
         * First 4 is values for wheels.
         * */

        if (pCurrentTargetLevels == NULL)
            continue;

        uint32_t average_delta = 0;
        for (portSHORT i = 0; i < WHEELS_COUNT; ++i)
        {
            int32_t diff = (adc_data.values[i] - pCurrentTargetLevels->wheels[i]);
            average_delta += abs(diff);
        }
        average_delta /= WHEELS_COUNT;

        AdcValue_t result_value = (AdcValue_t)average_delta;
        sendToQueueOverride(&adcAverageQueue, &result_value);
#endif

        DUMMY_BREAK;
    }

    deleteTask();
}


void vTelemetryTask( void *pvParameters )
{
    int n = 0;
    Diagnostic emptyDiagnostic;
    memset((void*)&emptyDiagnostic, 12, sizeof(Diagnostic));

    for(;;)
    {
        for (portSHORT i = 0; i< ADC_FIFO_SIZE; ++i)
        {
            readFromQueue(&adcValuesQueues[i], &(diagnostic.adc_values.values[i]));
        }

        // Array from 0 to 7, from "front left up" to "back right down" wheel.

        diagnostic.wheels_stats[0] = getPin(wheelPinsFL.upPinStatus);
        diagnostic.wheels_stats[1] = getPin(wheelPinsFL.downPinStatus);
        diagnostic.wheels_stats[2] = getPin(wheelPinsFR.upPinStatus);
        diagnostic.wheels_stats[3] = getPin(wheelPinsFR.downPinStatus);
        diagnostic.wheels_stats[4] = getPin(wheelPinsBL.upPinStatus);
        diagnostic.wheels_stats[5] = getPin(wheelPinsBL.downPinStatus);
        diagnostic.wheels_stats[6] = getPin(wheelPinsBR.upPinStatus);
        diagnostic.wheels_stats[7] = getPin(wheelPinsBR.downPinStatus);

        diagnostic.wheel_pins[0] = getPin(wheelPinsFL.upPin);
        diagnostic.wheel_pins[1] = getPin(wheelPinsFL.downPin);
        diagnostic.wheel_pins[2] = getPin(wheelPinsFR.upPin);
        diagnostic.wheel_pins[3] = getPin(wheelPinsFR.downPin);
        diagnostic.wheel_pins[4] = getPin(wheelPinsBL.upPin);
        diagnostic.wheel_pins[5] = getPin(wheelPinsBL.downPin);
        diagnostic.wheel_pins[6] = getPin(wheelPinsBR.upPin);
        diagnostic.wheel_pins[7] = getPin(wheelPinsBR.downPin);

        sendDiagnosticData(&diagnostic, sizeof(Diagnostic));

        if (++n == 5)
        {
            sendDiagnosticData(&emptyDiagnostic, sizeof(Diagnostic));

            n = 0;
        }

        delayTask(MS_TO_TICKS(100));

        DUMMY_BREAK;
    }

    deleteTask();
}


void vCompressorTask( void *pvParameters )
{
    // default value 2 seconds
    short compressorTimeoutSec = 3;
    bool isWorking = false;
    AdcValue_t levelValue = 0;

    for(;;)
    {
        // time delay before each check if compressor is not working.
        if (!isWorking)
            delayTask(MS_TO_TICKS(compressorTimeoutSec * 1000));

        if (!readFromQueueWithTimeout(&adcValuesQueues[COMPRESSOR_IDX], &levelValue, 0))
        {
            printError(QueueReadTimeoutErrorCode, "Timeout at compressor value reading from the queue");
            continue;
        }

        if (!isWorking && levelValue < cachedSettings.compressor_preasure_min)
        {
            isWorking = true;
            openPin(COMPRESSOR_HET_PIN);
        }
        else if (isWorking && levelValue > cachedSettings.compressor_preasure_max)
        {
            isWorking = false;
            closePin(COMPRESSOR_HET_PIN);
        }

        DUMMY_BREAK;
    }

    deleteTask();
}


void vWheelTask( void *pvParameters )
{
    const WheelPinsStruct wheelPins = *(WheelPinsStruct*)pvParameters;
    const WHEEL_IDX wheelNumber = wheelPins.wheel;
    const Queue wheelQueue = wheelsCommandsQueues[wheelNumber];

    void (*logicFunctionPointer)(WheelStatusStruct* wheelStatus) = executeWheelLogic;

    WheelStatusStruct wheelStatus =
    {
       .wheelPins = wheelPins,
       .wheelNumber = wheelNumber,
       .isWorking = false,
       .levelLimitValue = 0,
       .startTime = 0,
       .cmdType = UNKNOWN_COMMAND,
    };

    Command cmd;
    for( ;; )
    {
        /*
         * if 'isWorking' then don't wait for the next command, continue execution
         */
        boolean receivedCommand = popFromQueueWithTimeout(&wheelQueue, &cmd, wheelStatus.isWorking ? 0 : portMAX_DELAY);

        /*
        * New command received
        */
        if (receivedCommand)
        {
            initializeWheelStatus(&wheelStatus, &cmd);

            // actually never happens, but anyway ...
            if (cmd.commandType == UNKNOWN_COMMAND)
            {
                printError(UnknownCommandErrorCode, "Unknown command received in wheel task");
                continue; //loop
            }
        }

        logicFunctionPointer(&wheelStatus);

        switch (wheelStatus.cmdType)
        {
        case CMD_WHEEL_UP:
            upWheel(wheelStatus.wheelPins);
            break;
        case CMD_WHEEL_DOWN:
            downWheel(wheelStatus.wheelPins);
            break;  //switch
        case CMD_WHEEL_STOP:
            stopWheel(wheelStatus.wheelPins);
            break;
        }

        if (!wheelStatus.isWorking)
        {
            stopWheel(wheelStatus.wheelPins);
            resetWheelStatus(&wheelStatus);
        }

        DUMMY_BREAK;
    }

    deleteTask();
}


void vCommandHandlerTask( void *pvParameters )
{
    portCHAR receivedCommand[MAX_COMMAND_LEN] = {'\0'};
    for( ;; )
    {
        memset(receivedCommand, 0, MAX_COMMAND_LEN);

        if (popFromQueue(&commandsQueue, receivedCommand))
        {
            Command command = parseCommand(receivedCommand);
            sendToExecuteCommand(command);
        }
        else
        {
            printError(CommandsQueueErrorCode, "Could not receive a value from the queue.");
        }

        DUMMY_BREAK;
    }
    deleteTask();
}

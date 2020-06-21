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
#include "Utils/List.h"

#define DUMMY_BREAK if (0) break

static Queue commandsQueue;
static Queue memoryCommandsQueue;
static Queue wheelsCommandsQueues[WHEELS_COUNT];
static Queue adcValuesQueues[ADC_FIFO_SIZE]; // adc values have specific order, see ADCController.h(.c)

#ifndef SIMPLE_WHEEL_LOGIC
static Queue adcAverageQueue;
#endif

static LevelValues cachedLevels[LEVELS_COUNT];
static Settings cachedSettings;
static Diagnostic diagnostic;
static LevelValues* pCurrentTargetLevels = NULL;

static void vMemTask( void *pvParameters );
static void vCommandHandlerTask( void *pvParameters );
static void vADCUpdaterTask( void *pvParameters );
static void vTelemetryTask( void *pvParameters );
static void vCompressorTask( void *pvParameters );
static void vWheelTask( void *pvParameters );
static void vTimerCallbackFunction(xTimerHandle xTimer);

extern portSHORT helpHandler(Command *cmd);

/*=================================================*/
#pragma SWI_ALIAS(swiSwitchToMode, 1)

// Mode = 0x10 for user and 0x1F for system mode
extern void swiSwitchToMode(uint32 mode);

/*=================================================*/


/* ------------------------ Command Handlers ------------------------ */
static portSHORT wheelAutoCommandHandler(Command *cmd);
static portSHORT wheelCommandHandler(Command *cmd);
static portSHORT getVersionHandler(Command *cmd);
static portSHORT getBatVoltageHandler(Command *cmd);
static portSHORT getComprPressureHandler(Command *cmd);
static portSHORT levelsGetHandler(Command *cmd);
static portSHORT levelsSaveHandler(Command *cmd);
static portSHORT levelsShowHandler(Command *cmd);
static portSHORT levelsSaveMaxHandler(Command *cmd);
static portSHORT levelsSaveMinHandler(Command *cmd);
static portSHORT levelsGetMinHandler(Command *cmd);
static portSHORT levelsGetMaxHandler(Command *cmd);
static portSHORT memClearHandler(Command *cmd);
static portSHORT getComprMaxPressureHandler(Command *cmd);
static portSHORT getComprMinPressureHandler(Command *cmd);
static portSHORT setComprMinPressureHandler(Command *cmd);
static portSHORT setComprMaxPressureHandler(Command *cmd);
static portSHORT unknownCommandHandler(Command *cmd);

/* ------------------------------------------------------------------ */

bool tasksInit(ErrorHandler errHandler)
{
    memset(&diagnostic, 0, sizeof(Diagnostic));

    // TODO: make queues registration in separate module

    /*
     * Queues initializing
     * */
    createQueue(MAX_COMMANDS_QUEUE_LEN, MAX_COMMAND_LEN, &commandsQueue);

    portSHORT i = 0;
    for (; i < WHEELS_COUNT; ++i)
    {
        createQueue(1, sizeof(Command), wheelsCommandsQueues + i); // the only command for each wheel
    }

    for (i = 0; i < ADC_FIFO_SIZE; ++i)
    {
        createQueue(1, sizeof(AdcValue_t), adcValuesQueues + i); // the only value for each wheel
    }

    createQueue(3, sizeof(Command), &memoryCommandsQueue);
#ifndef SIMPLE_WHEEL_LOGIC
    createQueue(1, sizeof(AdcValue_t), &adcAverageQueue);
#endif

    /*
     *  Create tasks for commands receiving and handling
     */
    bool taskResult = true;

    taskResult &= createTask(vCommandHandlerTask, "CommandHandlerTask", NULL,
                             TASK_DEFAULT_PRIORITY);
    if (!taskResult)
    {
        errHandler();
    }

    /*
     *  Wheels tasks
     */
    taskResult &= createTask(vWheelTask, "WheelTaskFL", (void*) &wheelPinsFL,
                             TASK_DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskFR", (void*) &wheelPinsFR,
                             TASK_DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskBL", (void*) &wheelPinsBL,
                             TASK_DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskBR", (void*) &wheelPinsBR,
                             TASK_DEFAULT_PRIORITY);

    if (!taskResult)
    {
        errHandler();
    }

    // TODO: some parts of code can be moved to define for simplicity reading

    /*
     * Memory task
     */
    taskResult &= createTask(vMemTask, "MemTask", NULL, TASK_DEFAULT_PRIORITY);
    if (!taskResult)
    {
        errHandler();
    }

    /*
     * ADC converter task
     */
    taskResult &= createTask(vADCUpdaterTask, "ADCUpdater", NULL,
                             TASK_DEFAULT_PRIORITY);
    if (!taskResult)
    {
        errHandler();
    }

    /*
     * Compressor task
     */
    taskResult &= createTask(vCompressorTask, "CompressorTask", NULL,
                             TASK_DEFAULT_PRIORITY);
    if (!taskResult)
    {
        errHandler();
    }

    /*
     * Telemetry task
     */
    taskResult &= createTask(vTelemetryTask, "TelemetryTask", NULL,
                             TASK_DEFAULT_PRIORITY);
    if (!taskResult)
    {
        errHandler();
    }

    // temporary timer with priority 2
    taskResult &= createAndRunTimer("SuperTimer", MS_TO_TICKS(500),
                                    vTimerCallbackFunction);
    if (!taskResult)
    {
        errHandler();
    }

    taskResult &= protocol_init();

    return taskResult;
}


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
static bool getBatteryVoltage(long* const retVoltage)
{
    uint16 adcValue = 0;

    if (readFromQueueWithTimeout(&adcValuesQueues[BATTERY_IDX], &adcValue,
                                 MS_TO_TICKS(1000)))
    {
        *retVoltage = (portLONG) (adcValue * (5.0 / 255.0) * // convert to Volts, ADC 8 bit
                (147.0 / 47.0) *    // devider 4.7k/14.7k
                1000);              // milivolts

        return 0;
    }

    return QueueBatteryReadTimeoutErrorCode;
}

inline bool getWheelLevelValue(const portSHORT wheelNumber,
                               uint16 * const retLevel)
{
    return readFromQueueWithTimeout(&adcValuesQueues[wheelNumber], retLevel,
                                    MS_TO_TICKS(2000));
}
static int getCurrentWheelsLevelsValues(LevelValues* const retLevels)
{
    portSHORT i = 0;
    for (; i < WHEELS_COUNT; ++i)
    {
        if (!getWheelLevelValue(i, &(retLevels->wheels[i])))
        {
            return QueueWheelLevelReadTimeoutErrorCode;
        }
    }

    return 0;
}
static bool setCachedWheelLevel(uint8_t levelNumber, LevelValues values)
{
    bool rv = false;

    if (levelNumber < LEVELS_COUNT)
    {
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
static int getCompressorPressure(AdcValue_t* const retLevel)
{
    if (!readFromQueueWithTimeout(&adcValuesQueues[COMPRESSOR_IDX], retLevel,
                                  0))
    {
        return QueueCompressorReadTimeoutErrorCode;
    }
    return 0;
}

static void executeWheelLogic(WheelStatusStruct* wheelStatus)
{
    /*
     * Check wheel level
     *
     * Here we should check that we are not exceeded a needed level.
     * If it happens then stop wheel and wait for a new command.
     * If not - continue cycle execution.
     */

    AdcValue_t levelValue = 0;
    if (!readFromQueueWithTimeout(&adcValuesQueues[wheelStatus->wheelNumber],
                                  &levelValue, 0))
    {
        printError(QueueWheelLevelReadTimeoutErrorCode);
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
            printError(QueueAdcAverageReadTimeoutErrorCode);
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
            boolean shouldWheelUp = levelValue
                    < (wheelStatus->levelLimitValue - WHEELS_LEVELS_THRESHOLD);
            boolean shouldWheelDown = levelValue
                    > (wheelStatus->levelLimitValue + WHEELS_LEVELS_THRESHOLD);

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
        volatile portSHORT elapsedTimeSec = (getTickCount()
                - wheelStatus->startTime) / configTICK_RATE_HZ;
        wheelStatus->isWorking = elapsedTimeSec < wheelStatus->timeoutSec;
    }
}  // executeWheelLogic
static void initializeWheelStatus(WheelStatusStruct* wheelStatus, Command* cmd)
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
            wheelStatus->levelLimitValue =
                    cachedLevels[savedLevelNumber].wheels[wheelStatus->wheelNumber];
    }

    // check if there is a timeout specified (in seconds)
    if (cmd->argc >= 3)
    {
        portSHORT seconds = cmd->argv[2];
        wheelStatus->timeoutSec = seconds;
    }
}

static void resetWheelStatus(WheelStatusStruct* status)
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

static void vTimerCallbackFunction(xTimerHandle xTimer)
{
    togglePin(LED_1_HET_PIN);
}

/*=================================================*/

static void vMemTask(void *pvParameters)
{
    swiSwitchToMode(0x1F);

    List *list = createList();

    registerCommandHandler(list, CMD_LEVELS_SAVE_MAX, levelsSaveMaxHandler);
    registerCommandHandler(list, CMD_LEVELS_SAVE_MIN, levelsSaveMinHandler);
    registerCommandHandler(list, CMD_LEVELS_SAVE, levelsSaveHandler);
    registerCommandHandler(list, CMD_LEVELS_GET_MAX, levelsGetMaxHandler);
    registerCommandHandler(list, CMD_LEVELS_GET_MIN, levelsGetMinHandler);
    registerCommandHandler(list, CMD_LEVELS_GET, levelsGetHandler);
    registerCommandHandler(list, CMD_LEVELS_SHOW, levelsShowHandler);
    registerCommandHandler(list, CMD_MEM_CLEAR, memClearHandler);
    registerCommandHandler(list, CMD_GET_BATTERY, getBatVoltageHandler);
    registerCommandHandler(list, CMD_GET_COMPRESSOR_PRESSURE, getComprPressureHandler);
    registerCommandHandler(list, CMD_SET_COMPRESSOR_MAX_PRESSURE, setComprMaxPressureHandler);
    registerCommandHandler(list, CMD_SET_COMPRESSOR_MIN_PRESSURE, setComprMinPressureHandler);
    registerCommandHandler(list, CMD_GET_COMPRESSOR_MAX_PRESSURE, getComprMaxPressureHandler);
    registerCommandHandler(list, CMD_GET_COMPRESSOR_MIN_PRESSURE, getComprMinPressureHandler);

    initializeFEE();

    // update cached levels to actual values
    readLevels((void*) &cachedLevels);
    readSettings((void*) &cachedSettings);

    Command cmd;
    for (;;)
    {
        if (popFromQueue(&memoryCommandsQueue, &cmd))
        {
            int ret = executeCommand(list, &cmd, false);
            if (ret)
            {
                printError(ret);
            }
        }
        else
        {
            printError(MemoryReceiveQueueErrorCode);
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
static void vADCUpdaterTask(void *pvParameters)
{
    AdcDataValues adc_data;
    for (;;)
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

static void vTelemetryTask(void *pvParameters)
{
    int n = 0;
    Diagnostic emptyDiagnostic;
    memset((void*) &emptyDiagnostic, 12, sizeof(Diagnostic));

    for (;;)
    {
        for (portSHORT i = 0; i < ADC_FIFO_SIZE; ++i)
        {
            readFromQueue(&adcValuesQueues[i],
                          &(diagnostic.adc_values.values[i]));
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

static void vCompressorTask(void *pvParameters)
{
    short compressorDefaultTimeoutSec = 3;
    bool isWorking = false;
    AdcValue_t levelValue = 0;

    for (;;)
    {
        // time delay before each check if compressor is not working.
        if (!isWorking)
            delayTask(MS_TO_TICKS(compressorDefaultTimeoutSec * 1000));

        if (!readFromQueueWithTimeout(&adcValuesQueues[COMPRESSOR_IDX],
                                      &levelValue, 0))
        {
            printErrorStr(QueueCompressorReadTimeoutErrorCode, "Read timeout from compressor ADC value");
            continue;
        }

        if (!isWorking && levelValue < cachedSettings.compressor_preasure_min)
        {
            isWorking = true;
            openPin(COMPRESSOR_HET_PIN);
        }
        else if (isWorking
                && levelValue > cachedSettings.compressor_preasure_max)
        {
            isWorking = false;
            closePin(COMPRESSOR_HET_PIN);
        }

        DUMMY_BREAK;
    }

    deleteTask();
}

static void vWheelTask(void *pvParameters)
{
    const WheelPinsStruct wheelPins = *(WheelPinsStruct*) pvParameters;
    const WHEEL_IDX wheelNumber = wheelPins.wheel;
    const Queue wheelQueue = wheelsCommandsQueues[wheelNumber];

    void (*logicFunctionPointer)(
            WheelStatusStruct* wheelStatus) = executeWheelLogic;

    WheelStatusStruct wheelStatus =
            { .wheelPins = wheelPins, .wheelNumber = wheelNumber, .isWorking =
                      false,
              .levelLimitValue = 0, .startTime = 0, .cmdType = UNKNOWN_COMMAND, };

    Command cmd;
    for (;;)
    {
        /*
         * if 'isWorking' then don't wait for the next command, continue execution
         */
        boolean receivedCommand = popFromQueueWithTimeout(
                &wheelQueue, &cmd, wheelStatus.isWorking ? 0 : portMAX_DELAY);

        /*
         * New command received
         */
        if (receivedCommand)
        {
            initializeWheelStatus(&wheelStatus, &cmd);

            // actually never happens, but anyway ...
            if (cmd.commandType == UNKNOWN_COMMAND)
            {
                printError(UnknownCommandErrorCode);
                continue; // loop
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

static portSHORT envCommandHandler(Command *cmd)
{
    switch (cmd->commandType)
    {
    case CMD_GET_BATTERY:
        return getBatVoltageHandler(cmd);
    case CMD_GET_VERSION:
        return getVersionHandler(cmd);
    case CMD_HELP:
        return helpHandler(cmd);
    }

    return 1;
}

static portSHORT sendCommandToMemoryTaskHandler(Command *cmd)
{
    if (!sendToQueueWithTimeout(&memoryCommandsQueue, (void*) cmd, 0))
    {
        return MemorySendQueueErrorCode;
    }

    return 0;
}

static void vCommandHandlerTask(void *pvParameters)
{
    swiSwitchToMode(0x1F);

    portCHAR receivedCommand[MAX_COMMAND_LEN] = { '\0' };

    List *list = createList();

    registerCommandHandler(list, WHEEL_COMMAND_TYPE, wheelCommandHandler);
    registerCommandHandler(list, MEMORY_COMMAND_TYPE, sendCommandToMemoryTaskHandler);
    registerCommandHandler(list, ENV_COMMAND_TYPE, envCommandHandler);
    registerCommandHandler(list, UNKNOWN_COMMAND, unknownCommandHandler);

    swiSwitchToMode(0x10);

    for (;;)
    {
        memset(receivedCommand, 0, MAX_COMMAND_LEN);

        if (popFromQueue(&commandsQueue, receivedCommand))
        {
            Command command = parseCommand(receivedCommand);
            int ret = executeCommand(list, &command, true);
            if (ret)
            {
                printError(ret);
            }
        }
        else
        {
            printError(CommandsQueueErrorCode);
        }

        DUMMY_BREAK;
    }
    deleteTask();
}

/* ========== TODO: move to separate files ============= */

static portSHORT unknownCommandHandler(Command *cmd)
{
    printError(UnknownCommandErrorCode);
    return 0;
}

/*
 * Detect up or down based on current level values and comparing it with saved one
 * */
static portSHORT wheelAutoCommandHandler(Command *cmd)
{
    printSuccess();

    LevelValues currentLevels;
    if (cmd->argc > 0 && !getCurrentWheelsLevelsValues(&currentLevels))
    {
        portSHORT savedLevelNumber =
                cmd->argv[0] > LEVELS_COUNT ? 0 : cmd->argv[0];
        pCurrentTargetLevels = cachedLevels + savedLevelNumber;

        // translate to new command, where the first argument is a wheel number
        Command newCmd;
        portSHORT i = 0;
        for (; i < WHEELS_COUNT; ++i)
        {
            if (currentLevels.wheels[i] == pCurrentTargetLevels->wheels[i])
                continue;

            newCmd.commandType =
                    (currentLevels.wheels[i] < pCurrentTargetLevels->wheels[i]) ?
                            CMD_WHEEL_UP : CMD_WHEEL_DOWN;
            newCmd.argv[0] = i; // not used for 'auto', but level number should be at argv[1] anyway
            newCmd.argv[1] = savedLevelNumber;  // level number
            newCmd.argv[2] = WHEEL_TIMER_TIMEOUT_SEC; // default value for auto mode. For single mode timeout is in execution task.

            // check the timeout param (in seconds)
            // and override second argument if it is needed
            if (cmd->argc > 1)
            {
                newCmd.argv[2] = cmd->argv[1];
            }
            newCmd.argc = 3;

            sendToQueueOverride(&wheelsCommandsQueues[i], (void*) &newCmd); // always returns pdTRUE
        }
    }

    return 0;
}

static portSHORT wheelCommandHandler(Command *cmd)
{
    if (cmd->commandType == CMD_WHEEL_AUTO)
    {
        return wheelAutoCommandHandler(cmd);
    }

    // execute Up, Down or Stop command for ALL wheels if there is no arguments
    if (cmd->argc == 0)
    {
        printSuccess();

        // for all wheels
        portSHORT i = 0;
        for (; i < WHEELS_COUNT; ++i)
        {
            sendToQueueOverride(&wheelsCommandsQueues[i], (void*) cmd); // always returns pdTRUE
        }
    }
    // execute Up, Down or Stop command for SPECIFIC wheel if there is at least one parameter
    else
    {
        WHEEL_IDX wheelNo = (WHEEL_IDX) cmd->argv[0];
        if (wheelNo < WHEELS_COUNT)
        {
            printSuccess();
            sendToQueueOverride(&wheelsCommandsQueues[wheelNo], (void*) cmd); // always returns pdTRUE
        }
        else
        {
            printError(WrongWheelSpecifiedErrorCode);
        }
    }
    return 0;
}

static portSHORT getBatVoltageHandler(Command *cmd)
{
    portSHORT rv = 0;
    portLONG batteryVoltage = 0;

    rv = getBatteryVoltage(&batteryVoltage);
    if (!rv)
    {
        printSuccessNumber(batteryVoltage);
    }

    return rv;
}

static portSHORT getVersionHandler(Command *cmd)
{
    printSuccessString(APP_VERSION);
    return 0;
}

static portSHORT getComprPressureHandler(Command *cmd)
{
    int rv = 0;
    AdcValue_t level;

    rv = getCompressorPressure(&level);
    if (!rv)
    {
        printSuccessNumber(level);
    }

    return rv;
}

static portSHORT levelsGetHandler(Command *cmd)
{
    portSHORT levelNumber = (cmd->argc != 0) ? cmd->argv[0] : 0;
    if (levelNumber < LEVELS_COUNT && cmd->argc != 0)
    {
        printSuccessLevels(&(getCachedWheelLevels()[levelNumber]));
    }
    else
    {
        return WrongLevelSpecifiedErrorCode;
    }

    return 0;
}

static portSHORT levelsSaveHandler(Command *cmd)
{
    short levelNumber = (cmd->argc != 0) ? cmd->argv[0] : 0;
    if (levelNumber >= LEVELS_COUNT || cmd->argc == 0)
    {
        return WrongLevelSpecifiedErrorCode;
    }

    bool useDummyValue = (cmd->argc == 2);
    LevelValues currLevels;

    if (useDummyValue)
    {
        short i;
        for (i = 0; i< WHEELS_COUNT;++i)
        {
            currLevels.wheels[i] = cmd->argv[1];
        }
    }
    else
    {
        int ret = getCurrentWheelsLevelsValues(&currLevels);
        if (ret)
            return ret;
    }

    GLOBAL_SYNC_START;
        setCachedWheelLevel(levelNumber, currLevels);
        writeLevels((void*)getCachedWheelLevels());
    GLOBAL_SYNC_END;

    printSuccessLevels(&currLevels);

    return 0;
}

static portSHORT levelsShowHandler(Command *cmd)
{
    LevelValues currLevels;
    if (!getCurrentWheelsLevelsValues(&currLevels))
    {
        printSuccessLevels(&currLevels);
    }

    return 0;
}

static inline portSHORT saveLevels(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc, SETTING_TYPE type)
{
    LevelValues currLevel;
    if (!getCurrentWheelsLevelsValues(&currLevel))
    {
        GLOBAL_SYNC_START;
            uint8_t i;
            LevelValues *levelValues = (type == SettingMin) ? &(getSettings()->levels_values_min) : &(getSettings()->levels_values_max);
            for (i = 0; i < WHEELS_COUNT; ++i)
            {
                levelValues->wheels[i] = (argc==0)?currLevel.wheels[i]:argv[0];
            }
            writeSettings(getSettings());
        GLOBAL_SYNC_END;

        printSuccessLevels(levelValues);
    }

    return 0;
}

static portSHORT levelsSaveMinHandler(Command *cmd)
{
    return saveLevels(cmd->argv, cmd->argc, SettingMin);
}

static portSHORT levelsSaveMaxHandler(Command *cmd)
{
    return saveLevels(cmd->argv, cmd->argc, SettingMax);
}

static portSHORT levelsGetMinHandler(Command *cmd)
{
    printSuccessLevels(&(getSettings()->levels_values_min));

    return 0;
}

static portSHORT levelsGetMaxHandler(Command *cmd)
{
    printSuccessLevels(&(getSettings()->levels_values_max));

    return 0;
}

static portSHORT memClearHandler(Command *cmd)
{
    formatFEE();

    return 0;
}

static portSHORT getComprMaxPressureHandler(Command *cmd)
{
    printSuccessNumber(getSettings()->compressor_preasure_max);

    return 0;
}

static portSHORT getComprMinPressureHandler(Command *cmd)
{
    printSuccessNumber(getSettings()->compressor_preasure_min);

    return 0;
}

inline static portSHORT setComprPressure(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc, SETTING_TYPE type)
{
    AdcValue_t pressure;
    uint16_t *value = (type == SettingMin) ? &getSettings()->compressor_preasure_min : &getSettings()->compressor_preasure_max;
    if (!getCompressorPressure(&pressure))
    {
        GLOBAL_SYNC_START;
            *value = (argc==0) ? pressure : argv[0];
            writeSettings(getSettings());
        GLOBAL_SYNC_END;

        printSuccessNumber(*value);
    }

    return 0;
}

static portSHORT setComprMinPressureHandler(Command *cmd)
{
    return setComprPressure(cmd->argv, cmd->argc, SettingMin);
}

static portSHORT setComprMaxPressureHandler(Command *cmd)
{
    return setComprPressure(cmd->argv, cmd->argc, SettingMax);
}

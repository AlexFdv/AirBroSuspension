/** @file sys_main.c 
*   @brief Application main file
*   @date 07-July-2017
*   @version 04.07.00
*
*   This file contains an empty main function,
*   which can be used for the application.
*/

/* 
* Copyright (C) 2009-2016 Texas Instruments Incorporated - www.ti.com 
* 
* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


/* USER CODE BEGIN (0) */

#define DUMMY_BREAK if (0) break

#define VERSION "0.0.2"

/* USER CODE END */

/* Include Files */

#include "sys_common.h"

/* USER CODE BEGIN (1) */
#include "stdlib.h"
#include "stdio.h"

#include "gio.h"
#include "het.h"
#include "sci.h"
#include "adc.h"

#include <application/ADCController.h>
#include <application/FEEController.h>
#include <application/HetPinsController.h>
#include <application/SerialController.h>

#include "RtosWrapper/Rtos.h"
#include "RtosWrapper/RtosSemaphore.h"
#include "RtosWrapper/RtosQueue.h"

#include <application/Diagnostic.h>
#include <application/Settings.h>
#include <application/Levels.h>
#include <application/HetConstants.h>
#include <application/StringUtils.h>
#include <application/ConstantsCommon.h>
#include <application/WheelCommandStructs.h>
#include <application/CommandParser.h>


/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */

#pragma SWI_ALIAS(swiSwitchToMode, 1)

// Mode = 0x10 for user and 0x1F for system mode
extern void swiSwitchToMode ( uint32 mode );
void sendToExecuteCommand(WheelCommand);

static Queue commandsQueue;
static Queue memoryCommandsQueue;
static Queue wheelsCommandsQueues[WHEELS_COUNT];
static Queue adcValuesQueues[ADC_FIFO_SIZE];    // adc values have specific order, see ADCController.h(.c)
static Queue adcAverageQueue;

static Semaphore compressorSemaphore;
static portSHORT compressorTimeoutSec;

static LevelValues cachedLevels[LEVELS_COUNT];
static LevelValues* currentTargetLevels;
static Diagnostic diagnostic;

#define GLOBAL_SYNC_START suspendAllTasks()
#define GLOBAL_SYNC_END resumeAllTasks()

// assosiations with pins
const WheelPinsStruct wheelPinsFL = { FL_WHEEL, (portCHAR)FORWARD_LEFT_UP_PIN,
                                                (portCHAR)FORWARD_LEFT_DOWN_PIN,
                                                (portCHAR)FORWARD_LEFT_UP_STATUS_PIN,
                                                (portCHAR)FORWARD_LEFT_DOWN_STATUS_PIN };

const WheelPinsStruct wheelPinsFR = { FR_WHEEL, (portCHAR)FORWARD_RIGHT_UP_PIN,
                                                (portCHAR)FORWARD_RIGHT_DOWN_PIN,
                                                (portCHAR)FORWARD_RIGHT_UP_STATUS_PIN,
                                                (portCHAR)FORWARD_RIGHT_DOWN_STATUS_PIN };

const WheelPinsStruct wheelPinsBL = { BL_WHEEL, (portCHAR)BACK_LEFT_UP_PIN,
                                                (portCHAR)BACK_LEFT_DOWN_PIN,
                                                (portCHAR)BACK_LEFT_UP_STATUS_PIN,
                                                (portCHAR)BACK_LEFT_DOWN_STATUS_PIN };

const WheelPinsStruct wheelPinsBR = { BR_WHEEL, (portCHAR)BACK_RIGHT_UP_PIN,
                                                (portCHAR)BACK_RIGHT_DOWN_PIN,
                                                (portCHAR)BACK_RIGHT_UP_STATUS_PIN,
                                                (portCHAR)BACK_RIGHT_DOWN_STATUS_PIN };



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

inline bool getBatteryVoltage(portLONG* const retVoltage)
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

inline bool getCurrentWheelsLevelsValues(LevelValues* const retLevels)
{
    portSHORT i = 0;
    for (; i < WHEELS_COUNT; ++i)
    {
        if (!getWheelLevelValue(i, &(retLevels->wheels[i])))
        {
            printText("ERROR reading of level from mem task!!!");
            return false;
        }
    }

    return true;
}


inline void printLevels(const LevelValues* const levels)
{
    portSHORT i = 0;
    for (; i < WHEELS_COUNT; ++i)
    {
        printNumber(levels->wheels[i]);
        printText("\r\n");
    }
}

/*
 * Tasks implementation
*/

void vCommandHandlerTask( void *pvParameters )
{
    portCHAR receivedCommand[MAX_COMMAND_LEN] = {'\0'};
    for( ;; )
    {
        memset(receivedCommand, 0, MAX_COMMAND_LEN);

        if (popFromQueue(&commandsQueue, receivedCommand))
        {
            printText("Received the command: ");
            printText(receivedCommand);
            printText("\r\n");

            WheelCommand command = parseCommand(receivedCommand);
            sendToExecuteCommand(command);
        }
        else
        {
            printText("Could not receive a value from the queue.\r\n");
        }

        DUMMY_BREAK;
    }
    deleteTask();
}


void sendToExecuteCommand(WheelCommand cmd)
{
    if (cmd.Command == UNKNOWN_COMMAND)
    {
        printText("Unknown command received\r\n");
        return;
    }

    if ((cmd.Command & ENV_COMMAND_TYPE) == ENV_COMMAND_TYPE)
    {

        if (cmd.Command == CMD_DIAGNOSTIC)
        {
            sciSendData((uint8*)&diagnostic, (portSHORT)sizeof(Diagnostic));
        }

        if (cmd.Command == CMD_GET_VERSION)
        {
            printText(VERSION);
        }

        if (cmd.Command == CMD_GET_BATTERY)
        {
            portLONG batteryVoltage = 0;
            if (getBatteryVoltage(&batteryVoltage))
                printNumber(batteryVoltage);
            else
                printText("ERROR getting battery value.");
        }

        if (cmd.Command == CMD_COMPRESSOR)
        {
            compressorTimeoutSec = 5;   // default value for compressor 3 seconds
            if (cmd.argc > 0)
            {
                compressorTimeoutSec = cmd.argv[0];
            }
            giveSemaphore(&compressorSemaphore);
        }
    }

    if ((cmd.Command & WHEEL_COMMAND_TYPE) == WHEEL_COMMAND_TYPE)
    {
        // detect up or down based on current level values.
        if (cmd.Command == CMD_WHEEL_AUTO)
        {
            LevelValues levels;
            if (cmd.argc > 0 && getCurrentWheelsLevelsValues(&levels))
            {
                portSHORT savedLevelNumber = cmd.argv[0];
                currentTargetLevels = cachedLevels + savedLevelNumber;

                // translate to new command, where the first argument is a wheel number
                WheelCommand newCmd;
                portSHORT i = 0;
                for (; i< WHEELS_COUNT; ++i)
                {
                    if (levels.wheels[i] == currentTargetLevels->wheels[i])
                        continue;
                    newCmd.Command = (levels.wheels[i] < currentTargetLevels->wheels[i]) ? CMD_WHEEL_UP : CMD_WHEEL_DOWN;
                    newCmd.argv[0] = i;   // not used for 'auto', but level number should be at argv[1] anyway
                    newCmd.argv[1] = savedLevelNumber;  // level number
                    newCmd.argv[2] = WHEEL_TIMER_TIMEOUT_SEC;   // default value for auto mode. For single mode timeout is in execution task.

                    // check the timeout param (in seconds)
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
                sendToQueueOverride(&wheelsCommandsQueues[wheelNo], (void*)&cmd); // always returns pdTRUE
            }
        }
    }

    if ((cmd.Command & LEVELS_COMMAND_TYPE) == LEVELS_COMMAND_TYPE
            || (cmd.Command & SETTINGS_COMMAND_TYPE) == SETTINGS_COMMAND_TYPE)
    {
        // add command to the memory task queue: clear, save, print levels.
        // if arguments is empty, then default cell number is 0 (see vMemTask for details).
        if (!sendToQueueWithTimeout(&memoryCommandsQueue, (void*)&cmd, 0))
        {
            printText("Could not add memory command to the queue (it is full).\r\n");
        }
    }
}

void vMemTask( void *pvParameters )
{
    swiSwitchToMode(0x1F);

    initializeFEE();

    // update cached levels to actual values
    // TODO: clear cachedLevels to null values
    readLevels((void*)&cachedLevels);
    currentTargetLevels = NULL;

    WheelCommand cmd;
    for( ;; )
    {
        boolean result = popFromQueue(&memoryCommandsQueue,  &cmd);
        if (!result)
        {
            printText("ERROR in memory task!!!");
            continue;
        }

        if (cmd.Command == CMD_LEVELS_GET)
        {
            portSHORT levelNumber = (cmd.argc != 0) ? cmd.argv[0] : 0;
            levelNumber = (levelNumber >= LEVELS_COUNT) ? 0 : levelNumber;

            printLevels(&(cachedLevels[levelNumber]));
        }

        if (cmd.Command == CMD_LEVELS_SAVE)
        {
            portSHORT levelNumber = (cmd.argc != 0) ? cmd.argv[0] : 0;
            levelNumber = (levelNumber >= LEVELS_COUNT) ? 0 : levelNumber;

            LevelValues currLevel;
            if (getCurrentWheelsLevelsValues(&currLevel))
            {
                GLOBAL_SYNC_START;
                    cachedLevels[levelNumber] = currLevel;
                GLOBAL_SYNC_END;

                writeLevels((void*)&cachedLevels);

                printLevels(&currLevel);
                printText("levels saved to ");
                printNumber(levelNumber);
                printText("\r\n");
            }
        }

        if (cmd.Command == CMD_LEVELS_SHOW)
        {
            LevelValues currLevel;
            if (getCurrentWheelsLevelsValues(&currLevel))
            {
                printLevels(&currLevel);
            }
        }

        if (cmd.Command == CMD_MEM_CLEAR)
        {
            formatFEE();
        }

        DUMMY_BREAK;
    }

    deleteTask();
}

void vCompressorTask( void *pvParameters )
{
    for(;;)
    {
        takeSemaphore(&compressorSemaphore);

        printText("Compressor on");
        openPin(COMPRESSOR_HET_PIN);

        // temp delay for tests
        delayTask(MS_TO_TICKS(compressorTimeoutSec * 1000));

        printText("Compressor off");
        closePin(COMPRESSOR_HET_PIN);

        DUMMY_BREAK;
    }

    deleteTask();
}

inline void initializeWheelStatus(WheelStatusStruct* wheelStatus, WheelCommand* cmd)
{
    wheelStatus->isWorking = true;
    wheelStatus->startTime = xTaskGetTickCount();
    wheelStatus->cmdType = cmd->Command;
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

    static const TickType_t READ_LEVEL_TIMEOUT = MS_TO_TICKS(500);   // max timeout to wait level value from the queue. 500 ms.

    if (wheelStatus->levelLimitValue > 0)
    {
        AdcValue_t levelValue = 0;
        AdcValue_t average_delta = 0;

        if (readFromQueueWithTimeout(&adcValuesQueues[wheelStatus->wheelNumber], &levelValue, READ_LEVEL_TIMEOUT)
                && readFromQueueWithTimeout(&adcAverageQueue, &average_delta, 0))
        {
            if (average_delta < WHEELS_LEVELS_DEVIATION)
            {
                stopWheel(wheelStatus->wheelPins);
                /*
                 * do not change wheelStatus->isWorking here to false. It will setup to false automatically at timeout.
                 * */
            }
            else
            {
                if (levelValue < (wheelStatus->levelLimitValue - WHEELS_LEVELS_THRESHOLD))
                {
                    upWheel(wheelStatus->wheelPins);
                }
                else if (levelValue > (wheelStatus->levelLimitValue + WHEELS_LEVELS_THRESHOLD))
                {
                    downWheel(wheelStatus->wheelPins);
                }
                else
                {
                    stopWheel(wheelStatus->wheelPins);
                }
            }
        }
        else
        {
            /* If you see this message, then you have a problems... */
            printText("ERROR!!! Timeout at level value reading from the queue!!!");
        }
    }

    /*
     * Check timer
     */
    volatile portSHORT elapsedTimeSec = (xTaskGetTickCount() - wheelStatus->startTime) / configTICK_RATE_HZ;
    wheelStatus->isWorking = (elapsedTimeSec < wheelStatus->timeoutSec);

    /*
     * Check status pin
     * */
    if ((wheelStatus->cmdType == CMD_WHEEL_UP && getPin(wheelStatus->wheelPins.upPinStatus) != 1)
        || (wheelStatus->cmdType == CMD_WHEEL_DOWN && getPin(wheelStatus->wheelPins.downPinStatus) != 1))
    {
        wheelStatus->isWorking = false;

        /*
         * Array from 0 to 7, from "front left up" to "back right down" wheel.
         * */
        diagnostic.wheels_stats[wheelStatus->wheelNumber * 2] = getPin(wheelStatus->wheelPins.upPinStatus);
        diagnostic.wheels_stats[wheelStatus->wheelNumber * 2 + 1] = getPin(wheelStatus->wheelPins.downPinStatus);
    }
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

    WheelCommand cmd;
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

            switch (wheelStatus.cmdType) {
                case CMD_WHEEL_UP:
                    upWheel(wheelStatus.wheelPins);
                    break;  //switch
                case CMD_WHEEL_DOWN:
                    downWheel(wheelStatus.wheelPins);
                    break;  //switch
                case CMD_WHEEL_STOP:
                    stopWheel(wheelStatus.wheelPins);
                    resetWheelStatus(&wheelStatus);
                    continue;   //loop
                default:
                    // do nothing and goto cycle start
                    printText("Unknown command received");
                    continue; //loop
            }
        }

        logicFunctionPointer(&wheelStatus);

        /*
         * Do we need to stop working, or continue.
         * */
        if (!wheelStatus.isWorking)
        {
            stopWheel(wheelStatus.wheelPins);
            resetWheelStatus(&wheelStatus);
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

        for (portSHORT i = 0; i< ADC_FIFO_SIZE; ++i)
        {
            sendToQueueOverride(&adcValuesQueues[i], &(adc_data.values[i]));
        }

        /*
         * Calculate average deviation value of levels
         * First 4 is values for wheels.
         * */

        if (currentTargetLevels == NULL)
            continue;

        uint32_t average_delta = 0;
        for (portSHORT i = 0; i < WHEELS_COUNT; ++i)
        {
            int32_t diff = (adc_data.values[i] - currentTargetLevels->wheels[i]);
            average_delta += abs(diff);
        }
        average_delta /= WHEELS_COUNT;

        AdcValue_t result_value = (AdcValue_t)average_delta;
        sendToQueueOverride(&adcAverageQueue, &result_value);

        DUMMY_BREAK;
    }

    deleteTask();
}

void commandReceivedCallbackInterrupt(uint8* receivedCommand, short length)
{
    sendToQueueFromISR(&commandsQueue, receivedCommand);
}


void vTimerCallbackFunction(xTimerHandle xTimer)
{
    togglePin(LED_1_HET_PIN);
}

/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    _enable_IRQ();
    gioInit();
    initializeHetPins();
    initializeSci(&commandReceivedCallbackInterrupt);
    initializeADC();

    memset(&diagnostic, 0, sizeof(Diagnostic));

    createQueue(MAX_COMMANDS_QUEUE_LEN, MAX_COMMAND_LEN, &commandsQueue);

    portSHORT i = 0;
    for (; i< WHEELS_COUNT; ++i)
    {
        createQueue(1, sizeof(WheelCommand), wheelsCommandsQueues + i);  // the only command for each wheel
    }

    for (i = 0; i< ADC_FIFO_SIZE; ++i)
    {
        createQueue(1, sizeof(AdcValue_t), adcValuesQueues + i);  // the only value for each wheel
    }

    createQueue(3, sizeof(WheelCommand), &memoryCommandsQueue);
    createQueue(1, sizeof(AdcValue_t), &adcAverageQueue);

    /*
     *  Create tasks for commands receiving and handling
     */

    bool taskResult = true;
    compressorSemaphore = createBinarySemaphore();
    if (compressorSemaphore.handle == NULL)
        goto ERROR;


    taskResult &= createTask(vCommandHandlerTask, "CommandHanlderTask", NULL, TASK_DEFAULT_PRIORITY);
    if (!taskResult)
        goto ERROR;

    /*
     *  Wheels tasks
    */
    taskResult &= createTask(vWheelTask, "WheelTaskFL", (void*) &wheelPinsFL, TASK_DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskFR", (void*) &wheelPinsFR, TASK_DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskBL", (void*) &wheelPinsBL, TASK_DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskBR", (void*) &wheelPinsBR, TASK_DEFAULT_PRIORITY);

    if (!taskResult)
        goto ERROR;

    /*
     * Memory task
     */

    taskResult &= createTask(vMemTask, "MemTask", NULL, TASK_DEFAULT_PRIORITY);
    if (!taskResult)
        goto ERROR;

    /*
     * ADC converter task
     */
    taskResult &= createTask(vADCUpdaterTask, "ADCUpdater", NULL, TASK_DEFAULT_PRIORITY);
    if (!taskResult)
        goto ERROR;

    /*
     * Compressor task
     */
    taskResult &= createTask(vCompressorTask, "CompressorTask", NULL, TASK_DEFAULT_PRIORITY);

    // temporary timer
    taskResult &= createAndRunTimer("SuperTimer", MS_TO_TICKS(500), vTimerCallbackFunction);
    if (!taskResult)
        goto ERROR;

    printText("Controller started\r\n");
    vTaskStartScheduler();

ERROR:
    printText("Initialization error\r\n");
    while(1) DUMMY_BREAK;

    /* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */


/* USER CODE END */

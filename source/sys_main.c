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

#define VERSION "0.0.1"

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

#include "FreeRTOS.h"
#include "os_queue.h"

#include <application/Diagnostic.h>
#include <application/Settings.h>
#include <application/Levels.h>
#include <application/HetConstants.h>
#include <application/StringUtils.h>
#include <application/ConstantsCommon.h>
#include <application/WheelCommandStructs.h>


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
WheelCommand parseStringCommand(portCHAR command[MAX_COMMAND_LEN]);
void sendToExecuteCommand(WheelCommand);

Queue commandsQueue;
Queue memoryCommandsQueue;
Queue wheelsCommandsQueues[WHEELS_COUNT];
Queue adcValuesQueues[ADC_FIFO_SIZE];

Semaphore compressorSemaphore;

LevelValues cachedLevels[LEVELS_COUNT];
Diagnostic diagnostic;

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


/*
 * Tasks implementation
*/

void vCommandHandlerTask( void *pvParameters )
{
    portCHAR receivedCommand[MAX_COMMAND_LEN] = {'\0'};
    for( ;; )
    {
        memset(receivedCommand, 0, MAX_COMMAND_LEN);

        if (receiveFromQueue(&commandsQueue, receivedCommand))
        {
            printText("Received the command: ");
            printText(receivedCommand);
            printText("\r\n");

            WheelCommand command = parseStringCommand(receivedCommand);
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

void parseParams(char* strCmd, WheelCommand* const retCommand )
{
    char* str = strchr(strCmd, ' ');
    while (str != NULL)
    {
        ++str;
        if (isDigits(str, ' '))
        {
            portSHORT param = atoi(str);
            retCommand->argv[retCommand->argc] = param;
            retCommand->argc++;

            str = strchr(str, ' ');
        }
        else
        {
            break;
        }

        if (retCommand->argc >= COMMAND_ARGS_LIMIT)
        {
            break;
        }
    }
}

WheelCommand parseStringCommand(portCHAR command[MAX_COMMAND_LEN])
{
    WheelCommand parsedCommand =
    {
         UNKNOWN_COMMAND,
         {0},
         0
    };


    if (0 == strncmp(command, "diag", 4))
    {
        parsedCommand.Command = CMD_DIAGNOSTIC;
    }
    else
    if (0 == strncmp(command, "up", 2))
    {
        parsedCommand.Command = CMD_WHEEL_UP;
    }
    else
    if (0 == strncmp(command, "down", 4))
    {
        parsedCommand.Command = CMD_WHEEL_DOWN;
    }
    else
    if (0 == strncmp(command, "stop", 4))
    {
        parsedCommand.Command = CMD_WHEEL_STOP;
    }
    else
    if (0 == strncmp(command, "auto", 4))
    {
        parsedCommand.Command = CMD_WHEEL_AUTO;
    }
    else
    if (0 == strncmp(command, "lsave", 5))
    {
        parsedCommand.Command = CMD_LEVELS_SAVE;
    }
    else
    if (0 == strncmp(command, "lget", 4))
    {
        parsedCommand.Command = CMD_LEVELS_GET;
    }
    else
    if (0 == strncmp(command, "lshow", 5))
    {
        parsedCommand.Command = CMD_LEVELS_SHOW;
    }
    else
    if (0 == strncmp(command, "bat", 3))
    {
        parsedCommand.Command = CMD_GET_BATTERY;
    }
    else
    if (0 == strncmp(command, "compr", 5))
    {
        parsedCommand.Command = CMD_COMPRESSOR;
    }
    else
    if (0 == strncmp(command, "ver", 3))
    {
        parsedCommand.Command = CMD_GET_VERSION;
    }

    parseParams(command, &parsedCommand);

    return parsedCommand;
}

inline bool getWheelLevelValue(const portSHORT wheelNumber, uint16 * const retLevel)
{
    // clear to wait for the updated value from the ADCUpdater task.
    cleanQueue(&adcValuesQueues[wheelNumber]);

    return peekFromQueueWithTimeout(&adcValuesQueues[wheelNumber], retLevel, MS_TO_TICKS(2000));
}

inline bool getBatteryVoltage(portLONG* const retVoltage)
{
    uint16 adcValue = 0;

    if (peekFromQueueWithTimeout(&adcValuesQueues[BATTERY_IDX], &adcValue, MS_TO_TICKS(1000)))
    {
        *retVoltage = (portLONG)(adcValue *
                                (5.0 / 255.0) *     // convert to Volts, ADC 8 bit
                                (147.0 / 47.0) *    // devider 4.7k/14.7k
                                1000);              // milivolts

        return true;
    }

    return false;
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
                LevelValues savedLevels = cachedLevels[cmd.argv[0]];

                WheelCommand newCmd;
                portCHAR i = 0;
                for (; i< WHEELS_COUNT; ++i)
                {
                    newCmd.Command = (levels.wheels[i] < savedLevels.wheels[i]) ? CMD_WHEEL_UP : CMD_WHEEL_DOWN;
                    newCmd.argv[0] = i;   // not used for 'auto', but level number should be at argv[1] anyway
                    newCmd.argv[1] = cmd.argv[0];  // level number
                    newCmd.argc = 2;

                    sendToQueueOverride(&wheelsCommandsQueues[i], (void*)&newCmd); // always returns pdTRUE
                }
            }
        }
        // execute for all wheels if there is no arguments
        else if (cmd.argc == 0)
        {
            // for all wheels
            portCHAR i = 0;
            for (; i< WHEELS_COUNT; ++i)
            {
                sendToQueueOverride(&wheelsCommandsQueues[i], (void*)&cmd); // always returns pdTRUE
            }
        }
        // execute for specific wheel if there is at least one parameter
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

inline void printLevels(const LevelValues* const levels)
{
    portSHORT i = 0;
    for (; i < WHEELS_COUNT; ++i)
    {
        printNumber(levels->wheels[i]);
        printText("\r\n");
    }
}

void vMemTask( void *pvParameters )
{
    swiSwitchToMode(0x1F);

    initializeFEE();

    // update cached levels to actual values
    readLevels((void*)&cachedLevels);

    WheelCommand cmd;
    for( ;; )
    {
        boolean result = receiveFromQueue(&memoryCommandsQueue,  &cmd);
        if (!result)
        {
            printText("ERROR in memory task!!!");
        }

        if (cmd.Command == CMD_LEVELS_GET)
        {
            portSHORT levelNumber = cmd.argc != 0 ? cmd.argv[0] : 0;
            levelNumber = levelNumber >= LEVELS_COUNT ? 0 : levelNumber;

            printLevels(&(cachedLevels[levelNumber]));
        }

        if (cmd.Command == CMD_LEVELS_SAVE)
        {
            portSHORT levelNumber = cmd.argc != 0 ? cmd.argv[0] : 0;
            levelNumber = levelNumber >= LEVELS_COUNT ? 0 : levelNumber;

            LevelValues currLevel;
            if (getCurrentWheelsLevelsValues(&currLevel))
            {
                cachedLevels[levelNumber] = currLevel;
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
    const TickType timeDelay = MS_TO_TICKS(5000);
    for(;;)
    {
        takeSemaphore(&compressorSemaphore);

        printText("Compressor on");
        openPin(COMPRESSOR_HET_PIN);

        // temp delay for tests
        delayTask(timeDelay);

        printText("Compressor off");
        closePin(COMPRESSOR_HET_PIN);

        DUMMY_BREAK;
    }

    deleteTask();
}

inline void stopWheel(WheelPinsStruct wheelPins)
{
    closePin(wheelPins.upPin);
    closePin(wheelPins.downPin);
}

void initializeWheelStatus(WheelStatusStruct* wheelStatus, WheelCommand* cmd)
{
    wheelStatus->isWorking = true;
    wheelStatus->startTime = xTaskGetTickCount();
    wheelStatus->cmdType = cmd->Command;

    // check if there is a level limit value. If not, just up or down a wheel.
    if (cmd->argc > 1)
    {
        portSHORT number = cmd->argv[1];
        if (number < LEVELS_COUNT)
            wheelStatus->levelLimitValue = cachedLevels[number].wheels[wheelStatus->wheelNumber];
    }
}

void resetWheelStatus(WheelStatusStruct* status)
{
    status->isWorking = false;
    status->levelLimitValue = -1;
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

    const TickType READ_LEVEL_TIMEOUT = MS_TO_TICKS(500);   // max timeout to wait level value from the queue. 500 ms.

    if (wheelStatus->levelLimitValue >= 0)
    {
        uint16 levelValue = 0;

        if (peekFromQueueWithTimeout(&adcValuesQueues[wheelStatus->wheelNumber], &levelValue, READ_LEVEL_TIMEOUT))
        {
            wheelStatus->isWorking = (wheelStatus->cmdType == CMD_WHEEL_UP) ?
                                        (levelValue < wheelStatus->levelLimitValue) :
                                        (levelValue > wheelStatus->levelLimitValue);
        }
        else
        {
            printText("ERROR!!! Timeout at level value reading from the queue!!!");
        }
    }

    /*
     * Check timer
     */
    volatile portCHAR elapsedTimeSec = (xTaskGetTickCount() - wheelStatus->startTime) / configTICK_RATE_HZ;
    wheelStatus->isWorking = (elapsedTimeSec < WHEEL_TIMER_TIMEOUT_SEC);
}

void vWheelTask( void *pvParameters )
{
    const WheelPinsStruct wheelPins = *(WheelPinsStruct*)pvParameters;
    const portSHORT wheelNumber = (portSHORT)wheelPins.wheel;
    const Queue wheelQueue = wheelsCommandsQueues[wheelNumber];

    void (*logicFunctionPointer)(WheelStatusStruct* wheelStatus) = executeWheelLogic;

    WheelStatusStruct wheelStatus =
    {
       .wheelPins = wheelPins,
       .wheelNumber = wheelNumber,
       .isWorking = false,
       .levelLimitValue = -1,
       .startTime = 0,
       .cmdType = UNKNOWN_COMMAND
    };

    WheelCommand cmd;
    for( ;; )
    {
        /*
         * if 'isWorking' then don't wait for the next command, continue execution
         */
        boolean receivedCommand = receiveFromQueueWithTimeout(&wheelQueue, &cmd, wheelStatus.isWorking ? 0 : portMAX_DELAY);

        /*
        * New command received
        */
        if (receivedCommand)
        {
            initializeWheelStatus(&wheelStatus, &cmd);

            switch (wheelStatus.cmdType) {
                case CMD_WHEEL_UP:
                    stopWheel(wheelStatus.wheelPins);
                    openPin(wheelStatus.wheelPins.upPin);
                    break;  //switch
                case CMD_WHEEL_DOWN:
                    stopWheel(wheelStatus.wheelPins);
                    openPin(wheelStatus.wheelPins.downPin);
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
         * Check status pin
         * */
        if ((wheelStatus.cmdType == CMD_WHEEL_UP && getPin(wheelStatus.wheelPins.upPinStatus) != 1)
            || (wheelStatus.cmdType == CMD_WHEEL_DOWN && getPin(wheelStatus.wheelPins.downPinStatus) != 1))
        {
            wheelStatus.isWorking = false;

            /*
             * Array from 0 to 7, from "front left up" to "back right down" wheel.
             * */
            diagnostic.wheels_stats[wheelNumber * 2] = getPin(wheelStatus.wheelPins.upPinStatus);
            diagnostic.wheels_stats[wheelNumber * 2 + 1] = getPin(wheelStatus.wheelPins.downPinStatus);
        }

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
    // TODO: check the delay (remove it?)
    const TickType timeDelay = MS_TO_TICKS(10);  // 10 ms

    AdcDataValues adc_data;
    for( ;; )
    {
        getADCDataValues(&adc_data);

        for (portSHORT i = 0; i< ADC_FIFO_SIZE; ++i)
        {
            sendToQueueOverride(&adcValuesQueues[i], &(adc_data.values[i]));
        }

        delayTask(timeDelay);

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

    commandsQueue = createQueue(MAX_COMMANDS_QUEUE_LEN, MAX_COMMAND_LEN);

    portSHORT i = 0;
    for (; i< WHEELS_COUNT; ++i)
    {
        wheelsCommandsQueues[i] = createQueue(1, sizeof(WheelCommand));  // the only command for each wheel
    }

    for (i = 0; i< ADC_FIFO_SIZE; ++i)
    {
        adcValuesQueues[i] = createQueue(1, sizeof(ADC_VALUES_TYPE));  // the only value for each wheel
    }

    memoryCommandsQueue = createQueue(3, sizeof(WheelCommand));

    /*
     *  Create tasks for commands receiving and handling
     */

    bool taskResult = true;
    compressorSemaphore = createBinarySemaphore();
    if (compressorSemaphore.handle == NULL)
    {
        goto ERROR;
    }


    taskResult &= createTask(vCommandHandlerTask, "CommandHanlderTask", NULL, DEFAULT_PRIORITY);
    if (!taskResult)
    {
        goto ERROR;
    }

    /*
     *  Wheels tasks
    */
    taskResult &= createTask(vWheelTask, "WheelTaskFL", (void*) &wheelPinsFL, DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskFR", (void*) &wheelPinsFR, DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskBL", (void*) &wheelPinsBL, DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskBR", (void*) &wheelPinsBR, DEFAULT_PRIORITY);

    if (!taskResult)
    {
        goto ERROR;
    }

    /*
     * Memory task
     */

    taskResult &= createTask(vMemTask, "MemTask", NULL, DEFAULT_PRIORITY);
    if (!taskResult)
    {
        goto ERROR;
    }

    /*
     * ADC converter task
     */
    taskResult &= createTask(vADCUpdaterTask, "ADCUpdater", NULL, DEFAULT_PRIORITY);
    if (!taskResult)
    {
        goto ERROR;
    }

    /*
     * Compressor task
     */
    taskResult &= createTask(vCompressorTask, "CompressorTask", NULL, DEFAULT_PRIORITY);

    // temporary timer
    taskResult &= createAndRunTimer("SuperTimer", MS_TO_TICKS(500), vTimerCallbackFunction);
    if (!taskResult)
    {
        goto ERROR;
    }

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

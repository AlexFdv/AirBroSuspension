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

#define MS_TO_TICKS(x) ((x)/portTICK_RATE_MS)

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

#include "ADCController.h"
#include "FEEController.h"
#include "HetPinsController.h"
#include "SerialController.h"
#include "Commands.h"

#include "FreeRTOS.h"
#include "os_task.h"
#include "os_queue.h"
#include "os_semphr.h"

#include "Settings.h"
#include "Levels.h"
#include "HetConstants.h"
#include "StringUtils.h"
#include "ConstantsCommon.h"


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

xQueueHandle commandsQueueHandle;
xQueueHandle memoryCommandsQueueHandle;
xQueueHandle wheelsCommandsQueueHandles[WHEELS_COUNT];
xQueueHandle wheelsLevelsQueueHandles[WHEELS_COUNT];

const TickType_t READ_LEVEL_TIMEOUT = MS_TO_TICKS(500);   // max timeout to wait level value from the queue. 500 ms.

void printTextLin(const char* text);
void printNumberLin(const portSHORT number);
void printTextLin_ex(const char* text, short maxLen);

WheelCommand parseStringCommand(portCHAR command[MAX_COMMAND_LEN]);
void sendToExecuteCommand(WheelCommand);

typedef struct
{
    WHEEL wheel;
    portCHAR upPin;
    portCHAR downPin;
} WheelPinsStruct;

// assosiations with pins
const WheelPinsStruct wheelPinsFL = { FL_WHEEL, (portCHAR)FORWARD_LEFT_UP_PIN, (portCHAR)FORWARD_LEFT_DOWN_PIN };
const WheelPinsStruct wheelPinsFR = { FR_WHEEL, (portCHAR)FORWARD_RIGHT_UP_PIN, (portCHAR)FORWARD_RIGHT_DOWN_PIN };
const WheelPinsStruct wheelPinsBL = { BL_WHEEL, (portCHAR)BACK_LEFT_UP_PIN, (portCHAR)BACK_LEFT_DOWN_PIN };
const WheelPinsStruct wheelPinsBR = { BR_WHEEL, (portCHAR)BACK_RIGHT_UP_PIN, (portCHAR)BACK_RIGHT_DOWN_PIN };

LevelValues cachedLevels[LEVELS_COUNT];

/*
 * Tasks implementation
*/

void vCommandHandlerTask( void *pvParameters )
{
    portBASE_TYPE xStatus;

    portCHAR receivedCommand[MAX_COMMAND_LEN] = {'\0'};
    for( ;; )
    {
        memset(receivedCommand, 0, MAX_COMMAND_LEN);
        xStatus = xQueueReceive(commandsQueueHandle, receivedCommand, portMAX_DELAY);
        if (xStatus == pdTRUE)
        {
            printTextLin("Received the command: ");
            printTextLin_ex(receivedCommand, strlen(receivedCommand));
            printTextLin("\r\n");

            WheelCommand command = parseStringCommand(receivedCommand);
            sendToExecuteCommand(command);
        }
        else
        {
            printTextLin("Could not receive a value from the queue.\r\n");
        }

        DUMMY_BREAK;
    }
    vTaskDelete( NULL );
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

    if (0 == strncmp(command, "up", 2))
    {
        parsedCommand.Command = CMD_WHEEL_UP;
    }

    if (0 == strncmp(command, "down", 4))
    {
        parsedCommand.Command = CMD_WHEEL_DOWN;
    }

    if (0 == strncmp(command, "stop", 4))
    {
        parsedCommand.Command = CMD_WHEEL_STOP;
    }

    if (0 == strncmp(command, "auto", 4))
    {
        parsedCommand.Command = CMD_WHEEL_AUTO;
    }

    if (0 == strncmp(command, "lsave", 5))
    {
        parsedCommand.Command = CMD_LEVELS_SAVE;
    }

    if (0 == strncmp(command, "lget", 4))
    {
        parsedCommand.Command = CMD_LEVELS_GET;
    }

    if (0 == strncmp(command, "lshow", 5))
    {
        parsedCommand.Command = CMD_LEVELS_SHOW;
    }

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
    xQueueReset(wheelsLevelsQueueHandles[wheelNumber]);

    portBASE_TYPE xStatus = xQueuePeek(wheelsLevelsQueueHandles[wheelNumber], retLevel, MS_TO_TICKS(2000));

    return (xStatus == pdTRUE);
}

inline bool getCurrentWheelsLevelsValues(LevelValues* const retLevels)
{
    portSHORT i = 0;
    for (; i < WHEELS_COUNT; ++i)
    {
        if (!getWheelLevelValue(i, &(retLevels->wheels[i])))
        {
            printTextLin("ERROR reading of level from mem task!!!");
            return false;
        }
    }

    return true;
}

void sendToExecuteCommand(WheelCommand cmd)
{
    if (cmd.Command == UNKNOWN_COMMAND)
    {
        printTextLin("Unknown command received\r\n");
        return;
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

                    xQueueOverwrite(wheelsCommandsQueueHandles[i], (void*)&newCmd);  // always returns pdTRUE
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
                xQueueOverwrite(wheelsCommandsQueueHandles[i], (void*)&cmd);  // always returns pdTRUE
            }
        }
        // execute for specific wheel if there is at least one parameter
        else
        {
            WHEEL wheelNo = (WHEEL)cmd.argv[0];
            if (wheelNo < WHEELS_COUNT)
            {
                xQueueOverwrite(wheelsCommandsQueueHandles[wheelNo], (void*)&cmd);  // always returns pdTRUE
            }
        }
    }

    if ((cmd.Command & LEVELS_COMMAND_TYPE) == LEVELS_COMMAND_TYPE
            || (cmd.Command & SETTINGS_COMMAND_TYPE) == SETTINGS_COMMAND_TYPE)
    {
        // add command to the memory task queue: clear, save, print levels.
        // if arguments is empty, then default cell number is 0 (see vMemTask for details).
        portBASE_TYPE xStatus = xQueueSendToBack(memoryCommandsQueueHandle, (void*)&cmd, 0);
        if (xStatus != pdTRUE)
        {
            printTextLin("Could not add memory command to the queue (it is full).\r\n");
        }
    }

    if ((cmd.Command & SYSTEM_COMMAND_TYPE) == SYSTEM_COMMAND_TYPE)
    {
        printTextLin(VERSION);
    }
}

inline void printLevels(const LevelValues* const levels)
{
    portSHORT i = 0;
    for (; i < WHEELS_COUNT; ++i)
    {
        printNumberLin(levels->wheels[i]);
        printTextLin("\r\n");
    }
}

void vMemTask( void *pvParameters )
{
    swiSwitchToMode(0x1F);

    initializeFEE();

    // update cached levels to actual values
    readLevels((void*)&cachedLevels);

    TickType_t timeOut = portMAX_DELAY;
    WheelCommand cmd;
    for( ;; )
    {
        portBASE_TYPE xStatus = xQueueReceive(memoryCommandsQueueHandle, &cmd, timeOut);
        if (xStatus == pdFALSE)
        {
            printTextLin("ERROR in memory task!!!");
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
                printTextLin("levels saved to ");
                printNumberLin(levelNumber);
                printTextLin("\r\n");
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

    vTaskDelete( NULL );
}

inline void stopWheel(WheelPinsStruct wheelPins)
{
    closePin(wheelPins.upPin);
    closePin(wheelPins.downPin);
}

void vWheelTask( void *pvParameters )
{
    volatile portBASE_TYPE xStatus;
    volatile TickType_t timeOut = portMAX_DELAY;   // initial value to wait for the command. Then it will be 0 to not block the execution.

    WheelPinsStruct wheelPins = *(WheelPinsStruct*)pvParameters;

    TickType_t startTime = 0;
    portSHORT levelLimit = -1;

    volatile portSHORT wheelNumber = (portSHORT)wheelPins.wheel;
    xQueueHandle wheelQueueHandle = wheelsCommandsQueueHandles[wheelNumber];

    bool isWorking = false;
    WheelCommand cmd;
    for( ;; )
    {
        xStatus = xQueueReceive(wheelQueueHandle, &cmd, timeOut);
        /*if (xStatus == pdFALSE && timeOut == portMAX_DELAY)
        {
            printTextLin("FUCK ");
        }*/

        /*
        * Check for a new command
        */
        if (xStatus == pdTRUE)
        {
            isWorking = true;

            switch (cmd.Command) {
                case CMD_WHEEL_UP:
                    stopWheel(wheelPins);
                    openPin(wheelPins.upPin);
                    startTime = xTaskGetTickCount();
                    break;
                case CMD_WHEEL_DOWN:
                    stopWheel(wheelPins);
                    openPin(wheelPins.downPin);
                    startTime = xTaskGetTickCount();
                    break;
                case CMD_WHEEL_STOP:
                    isWorking = false;
                    break;
                default:
                    // do nothing and goto cycle start
                    printTextLin("Unknown command received");
                    continue;
            }

            // check if there is a level limit. If not, just up or down a wheel.
            if (cmd.argc > 1)
            {
                portSHORT number = cmd.argv[1];
                levelLimit = cachedLevels[number].wheels[wheelNumber];
            }

        } // end new command

        /*
         * Check wheel level
         *
         * Here we should check that we are not exceeded a needed level.
         * If it happens then stop wheel and wait for a new command.
         * If not - continue cycle execution.
         */

        if (levelLimit >= 0)
        {
            uint16 levelValue = 0;
            xStatus = xQueuePeek(wheelsLevelsQueueHandles[wheelNumber], &levelValue, READ_LEVEL_TIMEOUT);
            if (xStatus == pdTRUE)
            {
                isWorking = (cmd.Command == CMD_WHEEL_UP) ? (levelValue < levelLimit) : (levelValue > levelLimit);
            }
            else
            {
                printTextLin("ERROR!!! Timeout at level value reading from the queue!!!");
            }
        }

        /*
         * Check timer
         */
        volatile portCHAR elapsedTimeSec = (xTaskGetTickCount() - startTime)/configTICK_RATE_HZ;
        if (elapsedTimeSec >= WHEEL_TIMER_TIMEOUT_SEC)
        {
             isWorking = false;
        }

        // stop wheel if it is needed
        if (!isWorking)
        {
            stopWheel(wheelPins);
            timeOut = portMAX_DELAY;
            levelLimit = -1;   // we shouldn't forget to reset for the next incoming command
        }
        else
        {
            timeOut = 0;   // don't block task on queue next time and execute checks.
        }

        DUMMY_BREAK;
    }
    vTaskDelete( NULL );
}

/*
 * ADCUpdaterTask always tries to update ADC values by getADCValues.
 * A task pushes to queue an updated values.
 * Not sure that there is needed some delay. Anyway we should test it.
 * */

void vADCUpdaterTask( void *pvParameters )
{
    initializeADC();

    // TODO: check the delay (remove it?)
    const TickType_t timeDelay = MS_TO_TICKS(10);  // 10 ms

    adcData_t adc_data[WHEELS_COUNT];
    for( ;; )
    {
        getADCValues(&adc_data[0]);

        portSHORT i = 0;
        for (; i< WHEELS_COUNT; ++i)
        {
            xQueueOverwrite(wheelsLevelsQueueHandles[i], &(adc_data[i].value));  // always returns pdTRUE
        }

        vTaskDelay(timeDelay);
    }
}

void commandReceivedCallbackInterrupt(uint8* receivedCommand, short length)
{
    portBASE_TYPE *pxTaskWoken;
    xQueueSendToBackFromISR(commandsQueueHandle, receivedCommand, pxTaskWoken);
}

/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    _enable_IRQ();
    gioInit();
    initializeHetPins();
    initializeSci(&commandReceivedCallbackInterrupt);

    commandsQueueHandle = xQueueCreate(5, MAX_COMMAND_LEN);

    portSHORT i = 0;
    for (; i< WHEELS_COUNT; ++i)
    {
        wheelsCommandsQueueHandles[i] = xQueueCreate(1, sizeof(WheelCommand));  // the only command for each wheel
    }

    for (i = 0; i< WHEELS_COUNT; ++i)
    {
        wheelsLevelsQueueHandles[i] = xQueueCreate(1, sizeof(uint16));  // the only value for each wheel
    }

    memoryCommandsQueueHandle = xQueueCreate(3, sizeof(WheelCommand));

    /*
     *  Create tasks for commands receiving and handling
     */
    portBASE_TYPE taskResult = pdFAIL;

    taskResult = xTaskCreate(vCommandHandlerTask, "CommandHanlderTask", configMINIMAL_STACK_SIZE, (void*)NULL, DEFAULT_PRIORITY, NULL);
    if (taskResult != pdPASS)
    {
        goto ERROR;
    }

    /*
     *  Wheels tasks
    */
    taskResult = xTaskCreate(vWheelTask, "WheelTaskFL", configMINIMAL_STACK_SIZE, (void*)&wheelPinsFL, DEFAULT_PRIORITY, NULL);
    if (taskResult != pdPASS)
    {
        goto ERROR;
    }

    taskResult = xTaskCreate(vWheelTask, "WheelTaskFR", configMINIMAL_STACK_SIZE, (void*)&wheelPinsFR, DEFAULT_PRIORITY, NULL);
    if (taskResult != pdPASS)
    {
        goto ERROR;
    }

    taskResult = xTaskCreate(vWheelTask, "WheelTaskBL", configMINIMAL_STACK_SIZE, (void*)&wheelPinsBL, DEFAULT_PRIORITY, NULL);
    if (taskResult != pdPASS)
    {
        goto ERROR;
    }

    taskResult = xTaskCreate(vWheelTask, "WheelTaskBR", configMINIMAL_STACK_SIZE, (void*)&wheelPinsBR, DEFAULT_PRIORITY, NULL);
    if (taskResult != pdPASS)
    {
        goto ERROR;
    }

    /*
     * Memory task
     */
    taskResult = xTaskCreate(vMemTask, "MemTask", configMINIMAL_STACK_SIZE, NULL, DEFAULT_PRIORITY, NULL);
    if (taskResult != pdPASS)
    {
        goto ERROR;
    }

    /*
     * ADC converter task
     */
    taskResult = xTaskCreate(vADCUpdaterTask, "ADCUpdater", configMINIMAL_STACK_SIZE, NULL, DEFAULT_PRIORITY, NULL);
    if (taskResult != pdPASS)
    {
        goto ERROR;
    }

    printTextLin("Controller started\r\n");
    vTaskStartScheduler();

ERROR:
    printTextLin("Initialization error\r\n");
    while(1) DUMMY_BREAK;

    /* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */

inline void printNumberLin(const portSHORT number)
{
    char buff[10] = {'\0'};
    ltoa(number, buff);
    printTextLin(buff);
}

//prints the text with terminated null char
inline void printTextLin(const char* text)
{
    printTextLin_ex(text, strlen(text));
}

inline void printTextLin_ex(const char* text, const short maxLen)
{
    sciDisplayDataLin(text, maxLen);
}

/* USER CODE END */

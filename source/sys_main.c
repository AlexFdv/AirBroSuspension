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

#include "Levels.h"
#include "HetConstants.h"
#include "StringUtils.h"
#include "Constants.h"


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

void printText(const char* text);
void printNumber(const portSHORT number);
void printText_ex(const char* text, short maxLen);

WheelCommand parseStringCommand(portCHAR command[MAX_COMMAND_LEN]);
void addCommandToQueue(WheelCommand);

typedef struct
{
    WHEEL wheel;
    portCHAR upPin;
    portCHAR downPin;
} WheelPinsStruct;

// assosiations with pins
WheelPinsStruct wheelPinsFL = { FL_WHEEL, (portCHAR)FORWARD_LEFT_UP_PIN, (portCHAR)FORWARD_LEFT_DOWN_PIN };
WheelPinsStruct wheelPinsFR = { FR_WHEEL, (portCHAR)FORWARD_RIGHT_UP_PIN, (portCHAR)FORWARD_RIGHT_DOWN_PIN };
WheelPinsStruct wheelPinsBL = { BL_WHEEL, (portCHAR)BACK_LEFT_UP_PIN, (portCHAR)BACK_LEFT_DOWN_PIN };
WheelPinsStruct wheelPinsBR = { BR_WHEEL, (portCHAR)BACK_RIGHT_UP_PIN, (portCHAR)BACK_RIGHT_DOWN_PIN };

LevelValues cachedLevels[LEVELS_COUNT];

/*
 * Tasks implementation
*/

// TODO: move receiving to the interruption
void vCommandReceiverTask( void *pvParameters )
{
    portBASE_TYPE xStatus;
    portCHAR receivedCommand[MAX_COMMAND_LEN] = {'\0'};

    for( ;; )
    {
        memset(receivedCommand, 0, MAX_COMMAND_LEN);
        portSHORT receivedLen = 0;
        sciReceiveData(receivedCommand, &receivedLen, MAX_COMMAND_LEN);

        xStatus = xQueueSendToBack(commandsQueueHandle, receivedCommand, 0);
        if (xStatus != pdTRUE)
        {
            printText("Could not add value to the queue.\r\n");
        }

        taskYIELD();

        DUMMY_BREAK;
    }
    vTaskDelete( NULL );
}

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
            printText("Received the command: ");
            printText_ex(receivedCommand, strlen(receivedCommand));
            printText("\r\n");

            WheelCommand command = parseStringCommand(receivedCommand);
            addCommandToQueue(command);
        }
        else
        {
            printText("Could not receive a value from the queue.\r\n");
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

    // parse wheel command type
    if (0 == strncmp(command, "up", 2))
    {
        parsedCommand.Command = CMD_WHEEL_UP;
    }

    // parse wheel command type
    if (0 == strncmp(command, "down", 4))
    {
        parsedCommand.Command = CMD_WHEEL_DOWN;
    }

    // parse wheel command type
    if (0 == strncmp(command, "stop", 4))
    {
        parsedCommand.Command = CMD_WHEEL_STOP;
    }

    // additional action: parse wheel number
    if ((parsedCommand.Command & WHEEL_COMMAND_TYPE) == WHEEL_COMMAND_TYPE)
    {
        parseParams(command, &parsedCommand);
        return parsedCommand;
    }

    if (0 == strncmp(command, "lsave", 5))
    {
        parsedCommand.Command = CMD_LEVELS_SAVE;
        parseParams(command, &parsedCommand);
    }

    if (0 == strncmp(command, "lget", 4))
    {
        parsedCommand.Command = CMD_LEVELS_GET;
        parseParams(command, &parsedCommand);
    }

    if (0 == strncmp(command, "lshow", 5))
    {
        parsedCommand.Command = CMD_LEVELS_SHOW;
    }

    return parsedCommand;
}

void addCommandToQueue(WheelCommand cmd)
{
    if (cmd.Command == UNKNOWN_COMMAND)
    {
        printText("Unknown command received\r\n");
        return;
    }

    printText("Executing entered command...\r\n");

    if (cmd.Command & WHEEL_COMMAND_TYPE)
    {
        // open all wheels
        if (cmd.argc == 0)
        {
            portCHAR i = 0;
            for (; i< WHEELS_COUNT; ++i)
            {
                xQueueOverwrite(wheelsCommandsQueueHandles[i], (void*)&cmd);  // always returns pdTRUE
            }
        }
        else
        {
            WHEEL wheelNo = (WHEEL)cmd.argv[0];
            if (wheelNo < WHEELS_COUNT)
            {
                xQueueOverwrite(wheelsCommandsQueueHandles[wheelNo], (void*)&cmd);  // always returns pdTRUE
            }
        }
    }

    if (cmd.Command & LEVELS_COMMAND_TYPE)
    {
        // add command to the memory task queue: clear, save, print levels.
        portBASE_TYPE xStatus = xQueueSendToBack(memoryCommandsQueueHandle, (void*)&cmd, 0);
        if (xStatus != pdTRUE)
        {
            printText("Could not add memory command to the queue (it is full).\r\n");
        }
    }
}

inline bool getWheelLevelValue(const portSHORT wheelNumber, uint16 * const retLevel)
{
    // clear to wait for the updated value from the ADCUpdater task.
    xQueueReset(wheelsLevelsQueueHandles[wheelNumber]);

    startADCConversion(wheelNumber);
    portBASE_TYPE xStatus = xQueuePeek(wheelsLevelsQueueHandles[wheelNumber], retLevel, MS_TO_TICKS(5000));
    stopADCConversion(wheelNumber);

    return xStatus == pdTRUE;
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
            printText("ERROR in memory task!!!");
        }

        portSHORT levelNumber = cmd.argc != 0 ? cmd.argv[0] : 0;
        levelNumber = levelNumber >= LEVELS_COUNT ? 0 : levelNumber;

        if (cmd.Command == CMD_LEVELS_GET)
        {
            printLevels(&(cachedLevels[levelNumber]));
        }

        if (cmd.Command == CMD_LEVELS_SAVE)
        {
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

        if (cmd.Command == CMD_LEVELS_CLEAR)
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


const TickType_t READ_LEVEL_TIMEOUT = MS_TO_TICKS(500);   // max timeout to wait level value from the queue. 500 ms.

void vWheelTask( void *pvParameters )
{
    volatile portBASE_TYPE xStatus;
    volatile TickType_t timeOut = portMAX_DELAY;   // initial value to wait for the command. Then it will be 0 to not block the execution.

    WheelPinsStruct wheelPins = *(WheelPinsStruct*)pvParameters;

    TickType_t startTime = 0;

    volatile portSHORT wheelNumber = (portSHORT)wheelPins.wheel;
    xQueueHandle wheelQueueHandle = wheelsCommandsQueueHandles[wheelNumber];

    bool isWorking = false;
    WheelCommand cmd;
    for( ;; )
    {
        xStatus = xQueueReceive(wheelQueueHandle, &cmd, timeOut);
        if (xStatus == pdFALSE && timeOut == portMAX_DELAY)
        {
            printText("FUCK ");
        }

        /*
        * Check for a new command
        */
        if (xStatus == pdTRUE)
        {
            if (!isWorking)
            {
                isWorking = true;
                startADCConversion(wheelNumber);
            }

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
                    printText("Unknown command received");
                    continue;
            }
        } // end new command

        /*
         * Check wheel level
         *
         * Here we should check that we are not exceeded a needed level.
         * If it happens then stop wheel and wait for a new command.
         * If not - continue cycle execution.
         */

        uint16 levelValue = 0;
        xStatus = xQueuePeek(wheelsLevelsQueueHandles[wheelNumber], &levelValue, READ_LEVEL_TIMEOUT);
        if (xStatus == pdTRUE)
        {
            // just for tests
            if (levelValue < 200 || levelValue > 800)
            {
                //isWorking = false;
            }
        }
        else
        {
            printText("ERROR!!! Timeout at level value reading from the queue!!!");
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

            stopADCConversion(wheelNumber);
        }
        else
        {
            timeOut = 0;   // don't block task on queue next time and execute checks.
        }

        DUMMY_BREAK;
    }
    vTaskDelete( NULL );
}

void vADCUpdaterTask( void *pvParameters )
{
    initializeADC();

    const TickType_t timeDelay = MS_TO_TICKS(30);  // 30 ms

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


/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    gioInit();
    initializeHetPins();
    initializeSci();

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

    taskResult = xTaskCreate(vCommandReceiverTask, "CommandReceiverTask", configMINIMAL_STACK_SIZE, (void*)NULL, DEFAULT_PRIORITY, NULL);
    if (taskResult != pdPASS)
    {
        goto ERROR;
    }

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

    taskResult = xTaskCreate(vADCUpdaterTask, "ADCUpdater", configMINIMAL_STACK_SIZE, NULL, DEFAULT_PRIORITY, NULL);
    if (taskResult != pdPASS)
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

//prints the text with terminated null char
void printText(const char* text)
{
    printText_ex(text, strlen(text));
}

void printNumber(const portSHORT number)
{
    char buff[10] = {'\0'};
    ltoa(number, buff);
    printText(buff);
}

void printText_ex(const char* text, const short maxLen)
{
    sciDisplayData(text, maxLen);
}

/* USER CODE END */

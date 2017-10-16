/** @file sys_main.c 
*   @brief Application main file
*   @date 08-Feb-2017
*   @version 04.06.01
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

/* USER CODE END */

/* Include Files */

#include "sys_common.h"

/* USER CODE BEGIN (1) */
#include "stdlib.h"

#include "gio.h"
#include "het.h"
#include "sci.h"

#include "FEEController.h"
#include "HetPinsController.h"
#include "SerialController.h"
#include "Commands.h"

#include "FreeRTOS.h"
#include "os_task.h"
#include "os_queue.h"

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

xQueueHandle commandsQueueHandle;
xQueueHandle wheelsCommandsQueueHandles[WHEELS_COUNT];

void printText(const char* text);
void printText_ex(const char* text, short maxLen);

WheelCommand parseStringCommand(portCHAR command[MAX_COMMAND_LEN]);
void executeCommand(WheelCommand);

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

/*
 * Tasks implementation
*/

// TODO: move receiving to the interruption
void vCommandReceiverTask( void *pvParameters )
{
    portBASE_TYPE xStatus;
    portCHAR recevedCommand[MAX_COMMAND_LEN] = {'\0'};

    for( ;; )
    {
        memset(recevedCommand, 0, MAX_COMMAND_LEN);
        portSHORT receivedLen = 0;
        sciReceiveData(recevedCommand, &receivedLen, MAX_COMMAND_LEN);

        xStatus = xQueueSendToBack(commandsQueueHandle, recevedCommand, 0);
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
            printText_ex(receivedCommand, MAX_COMMAND_LEN);
            printText("\r\n");

            WheelCommand command = parseStringCommand(receivedCommand);
            executeCommand(command);
        }
        else
        {
            printText("Could not receive a value from the queue.\r\n");
        }

        DUMMY_BREAK;
    }
    vTaskDelete( NULL );
}

void vMemTask( void *pvParameters )
{
    initializeFEE();

    // TODO: remove, it is only for tests
    bool flag = true;
    for( ;; )
    {
        if (flag)
        {
            writeSyncFEE(1, 123);
            printText("Wrote value 123 once");
            flag = false;
        }
        DUMMY_BREAK;
    }

    vTaskDelete( NULL );
}

void vWheelTask( void *pvParameters )
{
    portBASE_TYPE xStatus;

    WheelPinsStruct wheelPins = *(WheelPinsStruct*)pvParameters;

    TickType_t timeOut = portMAX_DELAY;
    for( ;; )
    {
        WheelCommand cmd;
        xStatus = xQueueReceive(wheelsCommandsQueueHandles[wheelPins.wheel], &cmd, timeOut);
        timeOut = 0;

        // new command received
        if (xStatus == pdTRUE)
        {
            //close pins before a new command execution
            closePin(wheelPins.upPin);
            closePin(wheelPins.downPin);

            switch (cmd.Command) {
                case CMD_WHEEL_UP:
                    openPin(wheelPins.upPin);
                    break;
                case CMD_WHEEL_DOWN:
                     openPin(wheelPins.downPin);
                     break;
                case CMD_WHEEL_STOP:
                    // already closed
                    break;
                default:
                    printText("Unknown command received");
                    break;
            }
        } // end new command

        // here we should check that we do not exceeded a needed level
        // if it happens then stop task and wait for a new command

        timeOut = portMAX_DELAY;

        DUMMY_BREAK;
    }
    vTaskDelete( NULL );
}

WheelCommand parseStringCommand(portCHAR command[MAX_COMMAND_LEN])
{
    WheelCommand parsedCommand;

    //default values
    parsedCommand.Command = UNKNOWN_COMMAND;

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

    // post command parse action: parse wheel number
    if (parsedCommand.Command & WHEEL_COMMAND_TYPE)
    {
        // default value
        parsedCommand.argv[0] = ALL_WHEELS;
        parsedCommand.argc = 1;

        // get the wheel number
        char* wheelStr = strchr(command, ' ');
        if (wheelStr != NULL && isDigits(wheelStr + 1))
        {
            portSHORT wheelNo = atoi(wheelStr);
            if (wheelNo >= 0 && wheelNo < WHEELS_COUNT)
            {
                parsedCommand.argv[0] = wheelNo;
            }
        }
    }

    return parsedCommand;
}

void executeCommand(WheelCommand cmd)
{
    if (cmd.Command == UNKNOWN_COMMAND)
    {
        printText("Unknown command received\r\n");
        return;
    }

    printText("Executing entered command...\r\n");

    if (cmd.Command & WHEEL_COMMAND_TYPE && cmd.argc > 0)
    {
        WHEEL wheelNo = (WHEEL)cmd.argv[0];
        if (wheelNo == ALL_WHEELS)
        {
            portCHAR i = 0;
            for (; i< WHEELS_COUNT; ++i)
            {
                xQueueOverwrite(wheelsCommandsQueueHandles[i], (void*)&cmd);  // always returns pdTRUE
            }
        }
        else
        {
            xQueueOverwrite(wheelsCommandsQueueHandles[wheelNo], (void*)&cmd);
        }
    }

    if (cmd.Command & MEMORY_COMMAND_TYPE)
    {
        // add command to the memory task queue: clear, save, print levels.
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
void printText(const char* errorText)
{
    sciDisplayData(errorText, strlen(errorText));
}

void printText_ex(const char* errorText, short maxLen)
{
    sciDisplayData(errorText, maxLen);
}

/* USER CODE END */

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
/* USER CODE END */

/* Include Files */

#include "sys_common.h"

/* USER CODE BEGIN (1) */
#include "gio.h"
#include "het.h"
#include "sci.h"

#include "HetPinsController.h"
#include "SerialController.h"
#include "Commands.h"

#include "FreeRTOS.h"
#include "os_task.h"
#include "os_queue.h"

#include "HetConstants.h"
/* USER CODE END */


/* USER CODE BEGIN (2) */

xQueueHandle commandsQueueHandle;

void printText(const char* text);
void processCommand(Command* command);

void upFunction();
void downFunction();
void stopFunction();

void vCommandReceiverTask( void *pvParameters )
{
    portBASE_TYPE xStatus;
    portCHAR recevedCommand[MAX_COMMAND_LEN] = {'\0'};

    for( ;; )
    {
        memset(recevedCommand, 0, MAX_COMMAND_LEN);
        portSHORT receivedLen = 0;
        sciReceiveText(recevedCommand, &receivedLen, MAX_COMMAND_LEN);

        xStatus = xQueueSendToBack(commandsQueueHandle, recevedCommand, 0);
        if (xStatus != pdTRUE)
        {
            printText("Could not add value to the queue.\r\n");
        }

        taskYIELD();
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
            sciDisplayText(receivedCommand, MAX_COMMAND_LEN);
            printText("\r\n");

            Command* cmd = getCommand(receivedCommand);
            if (cmd != NULL)
                cmd->action();
            else
                printText("Unknown command \r\n");
        }
        else
        {
            printText("Could not receive a value from the queue.\r\n");
        }
    }
    vTaskDelete( NULL );
}
/*
void vSomeTask1( void *pvParameters )
{
    for( ;; )
    {
        openPin(FORWARD_LEFT_UP_PIN);
        openPin(FORWARD_LEFT_DOWN_PIN);
        openPin(FORWARD_RIGHT_UP_PIN);
        openPin(FORWARD_RIGHT_DOWN_PIN);

        openPin(BACK_LEFT_UP_PIN);
        openPin(BACK_LEFT_DOWN_PIN);
        openPin(BACK_RIGHT_UP_PIN);
        openPin(BACK_RIGHT_DOWN_PIN);

        vTaskDelay( 2000 / portTICK_RATE_MS);

        //sciDisplayText(TEXT2, TSIZE2);

        closePin(FORWARD_LEFT_UP_PIN);
        closePin(FORWARD_LEFT_DOWN_PIN);
        closePin(FORWARD_RIGHT_UP_PIN);
        closePin(FORWARD_RIGHT_DOWN_PIN);

        closePin(BACK_LEFT_UP_PIN);
        closePin(BACK_LEFT_DOWN_PIN);
        closePin(BACK_RIGHT_UP_PIN);
        closePin(BACK_RIGHT_DOWN_PIN);

        vTaskDelay( 2000 / portTICK_RATE_MS);

        printText("***Blink***\r\n");
    }
    vTaskDelete( NULL );
}*/

/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    gioInit();
    initializeHetPins();
    initializeSci();

    portBASE_TYPE taskResult = pdFAIL;
    bool regResult = true;

    commandsQueueHandle = xQueueCreate(5, MAX_COMMAND_LEN);

    taskResult = xTaskCreate(vCommandReceiverTask, "CommandReceiverTask", configMINIMAL_STACK_SIZE, (void*)NULL, 3, NULL);
    if (taskResult != pdPASS)
    {
        goto ERROR;
    }

    taskResult = xTaskCreate(vCommandHandlerTask, "CommandHanlderTask", configMINIMAL_STACK_SIZE, (void*)NULL, 3, NULL);
    if (taskResult != pdPASS)
    {
        goto ERROR;
    }

    printText("Controller started\r\n");


    regResult &= registerCommandByName("up", &upFunction);
    regResult &= registerCommandByName("down", &downFunction);
    regResult &= registerCommandByName("stop", &stopFunction);

    if (!regResult)
    {
        goto ERROR;
    }

    vTaskStartScheduler();

ERROR:
    printText("Initialization error\r\n");
    while(1) ;
/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */

void upFunction()
{
    printText("UpFunction \r\n");
    openPin(FORWARD_LEFT_UP_PIN);
    openPin(FORWARD_RIGHT_UP_PIN);
    openPin(BACK_LEFT_UP_PIN);
    openPin(BACK_RIGHT_UP_PIN);
}

void downFunction()
{
    printText("DownFunction \r\n");

    openPin(FORWARD_LEFT_DOWN_PIN);
    openPin(FORWARD_RIGHT_DOWN_PIN);
    openPin(BACK_LEFT_DOWN_PIN);
    openPin(BACK_RIGHT_DOWN_PIN);
}

void stopFunction()
{
    printText("StopFunction \r\n");

    closePin(FORWARD_LEFT_UP_PIN);
    closePin(FORWARD_LEFT_DOWN_PIN);
    closePin(FORWARD_RIGHT_UP_PIN);
    closePin(FORWARD_RIGHT_DOWN_PIN);

    closePin(BACK_LEFT_UP_PIN);
    closePin(BACK_LEFT_DOWN_PIN);
    closePin(BACK_RIGHT_UP_PIN);
    closePin(BACK_RIGHT_DOWN_PIN);
}

//prints the text with terminated null char
void printText(const char* errorText)
{
    sciDisplayText(errorText, strlen(errorText));
}

/* USER CODE END */

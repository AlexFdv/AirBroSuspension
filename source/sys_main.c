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
#include "HetPinsController.h"
#include "SerialController.h"
#include "sci.h"

#include "FreeRTOS.h"
#include "os_task.h"
#include "os_queue.h"

#include "HetConstants.h"
/* USER CODE END */


/* USER CODE BEGIN (2) */
void printText(const char* text);

// variables
#define MAX_COMMAND_LEN 10

xQueueHandle queueHandle;

typedef struct
{
    char command[MAX_COMMAND_LEN];
    short commandLen;
} Command;

void vCommandReceiverTask( void *pvParameters )
{
    portBASE_TYPE xStatus;
    char recevedCommand[MAX_COMMAND_LEN] = {'\0'};

    for( ;; )
    {
        Command cmd;

        short receivedLen = 0; // use portSHORT ???
        sciReceiveText(recevedCommand, &receivedLen, MAX_COMMAND_LEN);

        strncpy(cmd.command, recevedCommand, receivedLen);
        cmd.commandLen = receivedLen;

        xStatus = xQueueSendToBack(queueHandle, &cmd, 0);
        if (xStatus != pdPASS)
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
    for( ;; )
    {
        Command receivedCommand;
        xStatus = xQueueReceive(queueHandle, &receivedCommand, portMAX_DELAY);
        if (xStatus == pdPASS)
        {
            printText("Received the command: ");
            sciDisplayText(receivedCommand.command, receivedCommand.commandLen);
            printText("\r\n");
        }
        else
        {
            printText("Could not receive a value from the queue.\r\n");
        }
    }
    vTaskDelete( NULL );
}

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
}

/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    gioInit();
    initializeHetPins();
    initializeSci();

    portBASE_TYPE result = pdFAIL;
    queueHandle = xQueueCreate(5, sizeof(Command));

    result = xTaskCreate(vCommandReceiverTask, "CommandReceiverTask", configMINIMAL_STACK_SIZE, (void*)NULL, 3, NULL);
    if (result != pdPASS)
    {
        goto ERROR;
    }

    result = xTaskCreate(vCommandHandlerTask, "CommandHanlderTask", configMINIMAL_STACK_SIZE, (void*)NULL, 3, NULL);
    if (result != pdPASS)
    {
        goto ERROR;
    }

    //result = xTaskCreate(vSomeTask1, "BlinkTask1", configMINIMAL_STACK_SIZE, (void*)NULL, 3, NULL);
    //if (result != pdPASS)
    //{
    //    goto ERROR;
    //}

    printText("Controller started\r\n");

    vTaskStartScheduler();

ERROR:
    printText("Initialization error\r\n");
    while(1) ;
/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */

//prints the text with terminated null char
void printText(const char* errorText)
{
    sciDisplayText(errorText, strlen(errorText));
}

/* USER CODE END */

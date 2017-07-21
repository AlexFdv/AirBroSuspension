/** @file sys_main.c 
*   @brief Application main file
*   @date 05-Oct-2016
*   @version 04.06.00
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

#include "HetConstants.h"

// serial communication interface

#define TSIZE1 12
const uint32 TEXT1[TSIZE1] = { '\r', '\n', '|', '\t', 'C', 'H', '.', 'I', 'D', '=',
                        '0', 'x' };
#define TSIZE2 9
const uint32 TEXT2[TSIZE2] = { '\t', 'V', 'A', 'L', 'U', 'E', '=', '0', 'x' };
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */

#define LEN 10
void vSomeTask( void *pvParameters )
{
    for( ;; )
    {
        uint32 recevedArr[LEN] = {'\0'};
        uint32 receivedLen = 0;
        if (sciReceiveText(recevedArr, LEN, &receivedLen))
        {
            sciDisplayText(recevedArr, receivedLen);
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

        vTaskDelay( 10000 / portTICK_RATE_MS);

        //sciDisplayText(TEXT2, TSIZE2);

        closePin(FORWARD_LEFT_UP_PIN);
        closePin(FORWARD_LEFT_DOWN_PIN);
        closePin(FORWARD_RIGHT_UP_PIN);
        closePin(FORWARD_RIGHT_DOWN_PIN);

        closePin(BACK_LEFT_UP_PIN);
        closePin(BACK_LEFT_DOWN_PIN);
        closePin(BACK_RIGHT_UP_PIN);
        closePin(BACK_RIGHT_DOWN_PIN);

        vTaskDelay( 10000 / portTICK_RATE_MS);

        //sciDisplayText(TEXT1, TSIZE1);
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

    portBASE_TYPE result = pdFALSE;

    result = xTaskCreate(vSomeTask, "BlinkTask", configMINIMAL_STACK_SIZE, (void*)NULL, 3, NULL);
    result = xTaskCreate(vSomeTask1, "BlinkTask1", configMINIMAL_STACK_SIZE, (void*)NULL, 3, NULL);

    vTaskStartScheduler();

    while(1) ;
/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */


/* USER CODE END */

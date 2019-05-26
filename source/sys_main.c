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


/* USER CODE END */

/* Include Files */

#include "sys_common.h"

/* USER CODE BEGIN (1) */
#include <stdlib.h>
#include <stdio.h>

#include "gio.h"
#include "het.h"
#include "sci.h"
#include "adc.h"

#include "application/Tasks.h"
#include "application/HetConstants.h"
#include "application/ConstantsCommon.h"
#include "application/Wheels.h"
#include "application/HetPinsController.h"
#include "application/Protocol.h"
#include "application/SerialController.h"
//
#include "RtosWrapper/Rtos.h"

/* USER CODE END */



/* USER CODE BEGIN (2) */

void vTimerCallbackFunction(xTimerHandle xTimer)
{
    togglePin(LED_1_HET_PIN);
}


void criticalErrorHandler(void)
{
    //@todo: go to hardware error mode
    printError(UndefinedErrorCode, "Initialization error.");
    while(1)
    {
        if (0){
            break;
        }
    }
}



/* USER CODE END */



/** @fn int main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*   @return return code
*/
int main(void)
{
/* USER CODE BEGIN (3) */
    _enable_IRQ();
    gioInit();
    initializeHetPins();
    initializeSci(&commandReceivedCallback);
    initializeADC();

    if (!tasks_init())
    {
        criticalErrorHandler();
    }
    /*
     *  Create tasks for commands receiving and handling
     */

    bool taskResult = true;

    taskResult &= createTask(vCommandHandlerTask, "CommandHandlerTask", NULL, TASK_DEFAULT_PRIORITY);
    if (!taskResult)
    {
        criticalErrorHandler();
    }

    /*
     *  Wheels tasks
    */
    taskResult &= createTask(vWheelTask, "WheelTaskFL", (void*) &wheelPinsFL, TASK_DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskFR", (void*) &wheelPinsFR, TASK_DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskBL", (void*) &wheelPinsBL, TASK_DEFAULT_PRIORITY);
    taskResult &= createTask(vWheelTask, "WheelTaskBR", (void*) &wheelPinsBR, TASK_DEFAULT_PRIORITY);

    if (!taskResult)
    {
        criticalErrorHandler();
    }

    /*
     * Memory task
     */
    taskResult &= createTask(vMemTask, "MemTask", NULL, TASK_DEFAULT_PRIORITY);
    if (!taskResult)
    {
        criticalErrorHandler();
    }

    /*
     * ADC converter task
     */
    taskResult &= createTask(vADCUpdaterTask, "ADCUpdater", NULL, TASK_DEFAULT_PRIORITY);
    if (!taskResult)
    {
        criticalErrorHandler();
    }

    /*
     * Compressor task
     */
    taskResult &= createTask(vCompressorTask, "CompressorTask", NULL, TASK_DEFAULT_PRIORITY);
    if (!taskResult)
    {
        criticalErrorHandler();
    }

    /*
     * Telemetry task
     */
    taskResult &= createTask(vTelemetryTask, "TelemetryTask", NULL, TASK_DEFAULT_PRIORITY);
    if (!taskResult)
    {
        criticalErrorHandler();
    }

    // temporary timer with priority 2
    taskResult &= createAndRunTimer("SuperTimer", MS_TO_TICKS(500), vTimerCallbackFunction);
    if (!taskResult)
    {
        criticalErrorHandler();
    }

    printSuccessString("Controller started");
    vTaskStartScheduler();

    /* Should not be executed */
    criticalErrorHandler();

    /* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */


/* USER CODE END */

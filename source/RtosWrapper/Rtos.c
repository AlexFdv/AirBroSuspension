/*
 * Rtos.c
 *
 *  Created on: 18 вер. 2018 р.
 *      Author: Alex
 */

#include "Rtos.h"
#include "os_task.h"
#include "os_timer.h"

boolean CreateTask(TaskFunction_t task, const char * const pcTaskName, void * const pvParameters, unsigned long priority)
{
    portBASE_TYPE taskResult = pdFAIL;

    taskResult = xTaskCreate(task, pcTaskName, configMINIMAL_STACK_SIZE, pvParameters, priority, NULL);
    if (taskResult != pdPASS)
    {
        return false;
    }

    return true;
}

boolean CreateAndRunTimer(const char * const pcTimerName, const TickType_t xTimerPeriodInTicks, TimerCallbackFunction_t pxCallbackFunction )
{
    portBASE_TYPE taskResult = pdFAIL;
    xTimerHandle timerHandler = 0;

    timerHandler = xTimerCreate(pcTimerName, xTimerPeriodInTicks, pdTRUE, 0, pxCallbackFunction);
    if (timerHandler == 0)
    {
        return false;
    }

    taskResult = xTimerStart(timerHandler, 0);
    if (taskResult != pdPASS)
    {
        return false;
    }

    return true;
}


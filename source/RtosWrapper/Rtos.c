/*
 * Rtos.c
 *
 *  Created on: 18 вер. 2018 р.
 *      Author: Alex
 */

#include "Rtos.h"
#include "os_task.h"
#include "os_timer.h"

boolean createTask(TaskFunction_t task, const char * const pcTaskName, void * const pvParameters, unsigned long priority)
{
    portBASE_TYPE taskResult = pdFAIL;

    taskResult = xTaskCreate(task, pcTaskName, configMINIMAL_STACK_SIZE, pvParameters, priority, NULL);
    if (taskResult != pdPASS)
    {
        return false;
    }

    return true;
}

/*
 * Delete current task
 */
void deleteTask()
{
    vTaskDelete( NULL );
}

boolean createAndRunTimer(const char * const pcTimerName, const TickType_t xTimerPeriodInTicks, TimerCallbackFunction pxCallbackFunction )
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

void delayTask(TickType_t ticks)
{
    vTaskDelay(ticks);
}

void suspendAllTasks()
{
    vTaskSuspendAll();
}

boolean resumeAllTasks()
{
    return xTaskResumeAll() == pdTRUE;
}


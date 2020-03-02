/*
 * Rtos.h
 *
 *  Created on: 18 ���. 2018 �.
 *      Author: Alex
 */

#ifndef _APS_RTOSWRAPPER_RTOS_H_
#define _APS_RTOSWRAPPER_RTOS_H_

#include "sys_common.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "os_timer.h"

#define MS_TO_TICKS(x) ((x)/portTICK_RATE_MS)

#define GLOBAL_SYNC_START suspendAllTasks()
#define GLOBAL_SYNC_END resumeAllTasks()


typedef TimerCallbackFunction_t TimerCallbackFunction;

TickType_t getTickCount();
boolean createTask(TaskFunction_t task, const char * const pcTaskName, void * const pvParameters, unsigned long priority);
void deleteTask();
boolean createAndRunTimer(const char * const pcTimerName, const TickType_t xTimerPeriodInTicks, TimerCallbackFunction_t pxCallbackFunction );
void delayTask(TickType_t ticks);

void suspendAllTasks();
boolean resumeAllTasks();

#endif /* _APS_RTOSWRAPPER_RTOS_H_ */

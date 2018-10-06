/*
 * Rtos.h
 *
 *  Created on: 18 вер. 2018 р.
 *      Author: Alex
 */

#ifndef INCLUDE_RTOSWRAPPER_RTOS_H_
#define INCLUDE_RTOSWRAPPER_RTOS_H_

#include "sys_common.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "os_timer.h"

#include "Types.h"

#define MS_TO_TICKS(x) ((x)/portTICK_RATE_MS)

typedef TimerCallbackFunction_t TimerCallbackFunction;

TickType_t getTickCount();
boolean createTask(TaskFunction_t task, const char * const pcTaskName, void * const pvParameters, unsigned long priority);
void deleteTask();
boolean createAndRunTimer(const char * const pcTimerName, const TickType_t xTimerPeriodInTicks, TimerCallbackFunction_t pxCallbackFunction );
void delayTask(TickType_t ticks);

void suspendAllTasks();
boolean resumeAllTasks();

#endif /* INCLUDE_RTOSWRAPPER_RTOS_H_ */

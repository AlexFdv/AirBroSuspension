/*
 * Rtos.h
 *
 *  Created on: 18 ���. 2018 �.
 *      Author: Alex
 */

#ifndef INCLUDE_RTOSWRAPPER_RTOS_H_
#define INCLUDE_RTOSWRAPPER_RTOS_H_

#include "sys_common.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "os_timer.h"

boolean CreateTask(TaskFunction_t task, const char * const pcTaskName, void * const pvParameters, unsigned long priority);
boolean CreateAndRunTimer(const char * const pcTimerName, const TickType_t xTimerPeriodInTicks, TimerCallbackFunction_t pxCallbackFunction );

#endif /* INCLUDE_RTOSWRAPPER_RTOS_H_ */

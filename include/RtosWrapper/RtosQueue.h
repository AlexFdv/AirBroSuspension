/*
 * RtosQueue.h
 *
 *  Created on: 18 вер. 2018 р.
 *      Author: Alex
 */

#ifndef INCLUDE_RTOSWRAPPER_RTOSQUEUE_H_
#define INCLUDE_RTOSWRAPPER_RTOSQUEUE_H_

#include "sys_common.h"
#include "FreeRTOS.h"
#include "os_queue.h"

#include "Types.h"

typedef struct
{
    xQueueHandle handle;
} Queue;

Queue createQueue(const UBaseType uxQueueLength, const UBaseType uxItemSize);
void cleanQueue(const Queue* const queue);
void sendToQueueFromISR(const Queue* const queue, const void * const pvItemToQueue);
void sendToQueueOverride(const Queue* const queue, const void * const pvItemToQueue);
boolean sendToQueueWithTimeout(const Queue* const queue, void * const pvItemToQueue, TickType xTicksToWait);
boolean receiveFromQueue(const Queue* const queue, void * const pvBuffer);
boolean receiveFromQueueWithTimeout(const Queue* const queue, void * const pvBuffer, TickType xTicksToWait);
boolean peekFromQueueWithTimeout(const Queue* const queue, void * const pvBuffer, TickType xTicksToWait);


#endif /* INCLUDE_RTOSWRAPPER_RTOSQUEUE_H_ */

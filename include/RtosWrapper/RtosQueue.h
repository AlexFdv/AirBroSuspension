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
void cleanQueue(Queue queue);
void sendToQueueFromISR(Queue queue, const void * const pvItemToQueue);
void sendToQueueOverride(Queue queue, const void * const pvItemToQueue);
boolean sendToQueueWithTimeout(Queue queue, void * const pvItemToQueue, TickType xTicksToWait);
boolean receiveFromQueue(Queue queue, void * const pvBuffer);
boolean receiveFromQueueWithTimeout(Queue queue, void * const pvBuffer, TickType xTicksToWait);
boolean peekFromQueueWithTimeout(Queue queue, void * const pvBuffer, TickType xTicksToWait);


#endif /* INCLUDE_RTOSWRAPPER_RTOSQUEUE_H_ */

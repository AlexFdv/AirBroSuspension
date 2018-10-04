/*
 * RtosQueue.c
 *
 *  Created on: 18 ���. 2018 �.
 *      Author: Alex
 */

#include "RtosQueue.h"
#include "Types.h"
#include "os_task.h"
#include <stdlib.h>

Queue createQueue(const UBaseType uxQueueLength, const UBaseType uxItemSize)
{
    //@fixme: memory leak!!! The better way is to pass a pointer to the queue as an argument
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->handle = xQueueCreate(uxQueueLength, uxItemSize);
    return *queue;
}

void cleanQueue(const Queue* const queue)
{
    xQueueReset(queue->handle);
}

void sendToQueueFromISR(const Queue* const queue, const void * const pvItemToQueue)
{
    portBASE_TYPE pxTaskWoken = pdFALSE;
    xQueueSendToBackFromISR(queue->handle, pvItemToQueue, &pxTaskWoken);

    if (pxTaskWoken == pdTRUE)
    {
        taskYIELD();
    }
}

/*
 * Only for use with queues that have a length of one - so the queue is either
 * empty or full.
 *
 * Post an item on a queue.  If the queue is already full then overwrite the
 * value held in the queue.  The item is queued by copy, not by reference.
 */
void sendToQueueOverride(const Queue* const queue, const void * const pvItemToQueue)
{
    xQueueOverwrite(queue->handle, pvItemToQueue);
}

boolean sendToQueueWithTimeout(const Queue * const queue, void * const pvItemToQueue, TickType xTicksToWait)
{
    portBASE_TYPE xStatus = xQueueSendToBack(queue->handle, pvItemToQueue, xTicksToWait);

    return (xStatus == pdTRUE);
}

boolean popFromQueue(const Queue* const queue, void * const pvBuffer)
{
    portBASE_TYPE xStatus = xQueueReceive(queue->handle, pvBuffer, portMAX_DELAY);

    return (xStatus == pdTRUE);
}

boolean popFromQueueWithTimeout(const Queue* const queue, void * const pvBuffer, TickType xTicksToWait)
{
    portBASE_TYPE xStatus = xQueueReceive(queue->handle, pvBuffer, xTicksToWait);
    return (xStatus == pdTRUE);
}

boolean readFromQueueWithTimeout(const Queue* const queue, void * const pvBuffer, TickType xTicksToWait)
{
    portBASE_TYPE xStatus = xQueuePeek(queue->handle, pvBuffer, xTicksToWait);

    return (xStatus == pdTRUE);
}


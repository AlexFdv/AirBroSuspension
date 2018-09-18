/*
 * RtosQueue.c
 *
 *  Created on: 18 вер. 2018 р.
 *      Author: Alex
 */

#include "RtosQueue.h"
#include "Types.h"

Queue createQueue(const UBaseType uxQueueLength, const UBaseType uxItemSize)
{
    Queue queue;
    queue.handle = xQueueCreate(uxQueueLength, uxItemSize);
    return queue;
}

void cleanQueue(Queue queue)
{
    xQueueReset(queue.handle);
}

void sendToQueueFromISR(Queue queue, const void * const pvItemToQueue)
{
    portBASE_TYPE *pxTaskWoken = 0;
    xQueueSendToBackFromISR(queue.handle, pvItemToQueue, pxTaskWoken);
}

/*
 * Only for use with queues that have a length of one - so the queue is either
 * empty or full.
 *
 * Post an item on a queue.  If the queue is already full then overwrite the
 * value held in the queue.  The item is queued by copy, not by reference.
 */
void sendToQueueOverride(Queue queue, const void * const pvItemToQueue)
{
    xQueueOverwrite(queue.handle, pvItemToQueue);
}

boolean sendToQueueWithTimeout(Queue queue, void * const pvItemToQueue, TickType xTicksToWait)
{
    portBASE_TYPE xStatus = xQueueSendToBack(queue.handle, pvItemToQueue, xTicksToWait);

    return (xStatus == pdTRUE);
}

boolean receiveFromQueue(Queue queue, void * const pvBuffer)
{
    portBASE_TYPE xStatus = xQueueReceive(queue.handle, pvBuffer, portMAX_DELAY);

    return (xStatus == pdTRUE);
}

boolean receiveFromQueueWithTimeout(Queue queue, void * const pvBuffer, TickType xTicksToWait)
{
    portBASE_TYPE xStatus = xQueueReceive(queue.handle, pvBuffer, xTicksToWait);
    return (xStatus == pdTRUE);
}

boolean peekFromQueueWithTimeout(Queue queue, void * const pvBuffer, TickType xTicksToWait)
{
    portBASE_TYPE xStatus = xQueuePeek(queue.handle, pvBuffer, xTicksToWait);

    return (xStatus == pdTRUE);
}


/*
 * RtosSemaphore.c
 *
 *  Created on: 18 ���. 2018 �.
 *      Author: Alex
 */

#include "RtosSemaphore.h"

Semaphore createBinarySemaphore()
{
    Semaphore semaphore;
    semaphore.handle = xSemaphoreCreateBinary();
    return semaphore;
}

void takeSemaphore(Semaphore semaphore)
{
    xSemaphoreTake(semaphore.handle, portMAX_DELAY);
}

void giveSemaphore(Semaphore semaphore)
{
    xSemaphoreGive(semaphore.handle);
}

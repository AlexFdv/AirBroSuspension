/*
 * RtosSemaphore.c
 *
 *  Created on: 18 вер. 2018 р.
 *      Author: Alex
 */

#include "RtosSemaphore.h"

Semaphore createBinarySemaphore()
{
    Semaphore semaphore;
    semaphore.handle = xSemaphoreCreateBinary();
    return semaphore;
}

void takeSemaphore(const Semaphore* const semaphore)
{
    xSemaphoreTake(semaphore->handle, portMAX_DELAY);
}

void giveSemaphore(const Semaphore* const semaphore)
{
    xSemaphoreGive(semaphore->handle);
}

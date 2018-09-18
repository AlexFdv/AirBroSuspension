/*
 * RtosSemaphore.h
 *
 *  Created on: 18 вер. 2018 р.
 *      Author: Alex
 */

#ifndef INCLUDE_RTOSWRAPPER_RTOSSEMAPHORE_H_
#define INCLUDE_RTOSWRAPPER_RTOSSEMAPHORE_H_

#include "FreeRTOS.h"
#include "os_semphr.h"

typedef struct
{
    xSemaphoreHandle handle;
} Semaphore;

Semaphore createBinarySemaphore();
void takeSemaphore(const Semaphore* const semaphore);
void giveSemaphore(const Semaphore* const semaphore);

#endif /* INCLUDE_RTOSWRAPPER_RTOSSEMAPHORE_H_ */

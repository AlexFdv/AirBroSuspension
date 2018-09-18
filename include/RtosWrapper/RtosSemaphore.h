/*
 * RtosSemaphore.h
 *
 *  Created on: 18 ���. 2018 �.
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
void takeSemaphore(Semaphore semaphore);
void giveSemaphore(Semaphore semaphore);

#endif /* INCLUDE_RTOSWRAPPER_RTOSSEMAPHORE_H_ */

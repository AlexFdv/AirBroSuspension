/*
 * Tasks.h
 *
 *  Created on: May 26, 2019
 *      Author: oleg
 */

#ifndef INCLUDE_APPLICATION_TASKS_H_
#define INCLUDE_APPLICATION_TASKS_H_

#include <stdbool.h>
#include <stdint.h>

#include "ADCController.h"

bool tasks_init(void);

void vMemTask( void *pvParameters );
void vCommandHandlerTask( void *pvParameters );
void vADCUpdaterTask( void *pvParameters );
void vTelemetryTask( void *pvParameters );
void vCompressorTask( void *pvParameters );
void vWheelTask( void *pvParameters );

void commandReceivedCallback(uint8_t* receivedCommand, short length);

//@todo: separate these functions to another unit
bool getBatteryVoltage(long* const retVoltage);
bool getCompressorPressure(AdcValue_t* const retLevel);

#endif /* INCLUDE_APPLICATION_TASKS_H_ */
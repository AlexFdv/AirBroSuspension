/*
 * Wheels.h
 *
 *  Created on: May 26, 2019
 *      Author: oleg
 */

#ifndef INCLUDE_APPLICATION_WHEELS_H_
#define INCLUDE_APPLICATION_WHEELS_H_

#include "../RtosWrapper/Rtos.h" //todo: avoid FreeRTOS types
#include "ADCController.h"
#include "CommandStructs.h"

// it is better don't change the values, or find usages of it.
typedef enum
{
    FL_WHEEL = 0,  // front left
    FR_WHEEL = 1,  // front right
    BL_WHEEL = 2,  // back left
    BR_WHEEL = 3   // back right
} WHEEL_IDX;


typedef struct
{
    WHEEL_IDX wheel;
    portCHAR upPin;
    portCHAR downPin;
    portCHAR upPinStatus;
    portCHAR downPinStatus;
} WheelPinsStruct;


typedef struct
{
    bool isWorking;
    WHEEL_IDX wheelNumber;
    portSHORT timeoutSec;
    AdcValue_t levelLimitValue;
    WheelPinsStruct wheelPins;
    COMMAND_TYPE cmdType;
    TickType_t startTime;
} WheelStatusStruct;


// associations with pins
extern const WheelPinsStruct wheelPinsFL;
extern const WheelPinsStruct wheelPinsFR;
extern const WheelPinsStruct wheelPinsBL;
extern const WheelPinsStruct wheelPinsBR;



#endif /* INCLUDE_APPLICATION_WHEELS_H_ */

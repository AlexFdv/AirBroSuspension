/*
 * wheels.c
 *
 *  Created on: May 26, 2019
 *      Author: oleg
 */

#include "application/Wheels.h"
#include "application/HetConstants.h"

// associations with pins
const WheelPinsStruct wheelPinsFL = { FL_WHEEL, (portCHAR)FORWARD_LEFT_UP_PIN,
                                                (portCHAR)FORWARD_LEFT_DOWN_PIN,
                                                (portCHAR)FORWARD_LEFT_UP_STATUS_PIN,
                                                (portCHAR)FORWARD_LEFT_DOWN_STATUS_PIN };

const WheelPinsStruct wheelPinsFR = { FR_WHEEL, (portCHAR)FORWARD_RIGHT_UP_PIN,
                                                (portCHAR)FORWARD_RIGHT_DOWN_PIN,
                                                (portCHAR)FORWARD_RIGHT_UP_STATUS_PIN,
                                                (portCHAR)FORWARD_RIGHT_DOWN_STATUS_PIN };

const WheelPinsStruct wheelPinsBL = { BL_WHEEL, (portCHAR)BACK_LEFT_UP_PIN,
                                                (portCHAR)BACK_LEFT_DOWN_PIN,
                                                (portCHAR)BACK_LEFT_UP_STATUS_PIN,
                                                (portCHAR)BACK_LEFT_DOWN_STATUS_PIN };

const WheelPinsStruct wheelPinsBR = { BR_WHEEL, (portCHAR)BACK_RIGHT_UP_PIN,
                                                (portCHAR)BACK_RIGHT_DOWN_PIN,
                                                (portCHAR)BACK_RIGHT_UP_STATUS_PIN,
                                                (portCHAR)BACK_RIGHT_DOWN_STATUS_PIN };


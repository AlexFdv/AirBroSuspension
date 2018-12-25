/*
 * WheelStructs.h
 *
 *  Created on: 19 ρεπο. 2018 π.
 *      Author: Alex
 */

#ifndef INCLUDE_APPLICATION_WHEELCOMMANDSTRUCTS_H_
#define INCLUDE_APPLICATION_WHEELCOMMANDSTRUCTS_H_

#include "ConstantsCommon.h"
#include "ADCController.h"
#include "os_portmacro.h"

typedef enum
{
    UNKNOWN_COMMAND = 0b00000000,

    WHEEL_COMMAND_TYPE = 0b10000000,
    CMD_WHEEL_UP   = WHEEL_COMMAND_TYPE | 0b0001,
    CMD_WHEEL_DOWN = WHEEL_COMMAND_TYPE | 0b0010,
    CMD_WHEEL_STOP = WHEEL_COMMAND_TYPE | 0b0100,
    CMD_WHEEL_AUTO = WHEEL_COMMAND_TYPE | 0b1000,

    LEVELS_COMMAND_TYPE = 0b01000000,
    CMD_MEM_CLEAR       = LEVELS_COMMAND_TYPE | 0b0001,
    CMD_LEVELS_SAVE     = LEVELS_COMMAND_TYPE | 0b0010,
    CMD_LEVELS_SAVE_MAX = LEVELS_COMMAND_TYPE | 0b0011,
    CMD_LEVELS_SAVE_MIN = LEVELS_COMMAND_TYPE | 0b0100,
    CMD_LEVELS_GET      = LEVELS_COMMAND_TYPE | 0b0101,
    CMD_LEVELS_GET_MAX  = LEVELS_COMMAND_TYPE | 0b0110,
    CMD_LEVELS_GET_MIN  = LEVELS_COMMAND_TYPE | 0b0111,
    CMD_LEVELS_SHOW     = LEVELS_COMMAND_TYPE | 0b1000,

    SETTINGS_COMMAND_TYPE = 0b00100000,
    CMD_SAVE_SETTING_PREASURE_MAX = SETTINGS_COMMAND_TYPE | 0b0001,
    CMD_SAVE_SETTING_PREASURE_MIN = SETTINGS_COMMAND_TYPE | 0b0010,

    ENV_COMMAND_TYPE = 0b00010000,
    CMD_GET_VERSION  = ENV_COMMAND_TYPE | 0b0001,
    CMD_GET_BATTERY = ENV_COMMAND_TYPE | 0b0010,
    //CMD_COMPRESSOR = ENV_COMMAND_TYPE | 0b0100,
    CMD_GET_COMPRESSOR_PRESSURE = ENV_COMMAND_TYPE | 0b1000,
    CMD_SET_COMPRESSOR_MAX_PRESSURE = ENV_COMMAND_TYPE | 0b1001,
    CMD_SET_COMPRESSOR_MIN_PRESSURE = ENV_COMMAND_TYPE | 0b1010,
    //CMD_DIAGNOSTIC = ENV_COMMAND_TYPE | 0b1000
} COMMAND_TYPE;

typedef struct
{
    COMMAND_TYPE commandType;                  // command type, such as wheel command (up, down, stop), memory command (save, get, etc).
    portSHORT argv[COMMAND_ARGS_LIMIT];
    portCHAR argc;
} Command;

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

#endif /* INCLUDE_APPLICATION_WHEELCOMMANDSTRUCTS_H_ */

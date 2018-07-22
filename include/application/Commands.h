/*
 * Commands.h
 *
 *  Created on: 27 ����. 2017 �.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_COMMANDS_H_
#define SOURCE_INCLUDES_COMMANDS_H_

#include "../Application/ConstantsCommon.h"
#include "sys_common.h"
#include "string.h"
#include "FreeRTOS.h"

// TODO: move to bitmask, but change getting of wheel using these number from the array(!!!)
typedef enum
{
    FL_WHEEL = 0,  // front left
    FR_WHEEL = 1,  // front right
    BL_WHEEL = 2,  // back left
    BR_WHEEL = 3   // back right
} WHEEL;

typedef enum
{
    UNKNOWN_COMMAND = 0b00000000,

    WHEEL_COMMAND_TYPE = 0b10000000,
    CMD_WHEEL_UP   = WHEEL_COMMAND_TYPE | 0b0001,
    CMD_WHEEL_DOWN = WHEEL_COMMAND_TYPE | 0b0010,
    CMD_WHEEL_STOP = WHEEL_COMMAND_TYPE | 0b0100,
    CMD_WHEEL_AUTO = WHEEL_COMMAND_TYPE | 0b1000,

    LEVELS_COMMAND_TYPE = 0b01000000,
    CMD_MEM_CLEAR    = LEVELS_COMMAND_TYPE | 0b0001,
    CMD_LEVELS_SAVE  = LEVELS_COMMAND_TYPE | 0b0010,
    CMD_LEVELS_GET   = LEVELS_COMMAND_TYPE | 0b0100,
    CMD_LEVELS_SHOW  = LEVELS_COMMAND_TYPE | 0b1000,

    SETTINGS_COMMAND_TYPE = 0b00100000,
    CMD_SAVE_SETTING_PREASURE_MAX = SETTINGS_COMMAND_TYPE | 0b0001,
    CMD_SAVE_SETTING_PREASURE_MIN = SETTINGS_COMMAND_TYPE | 0b0010,

    SYSTEM_COMMAND_TYPE = 0b00010000,
    CMD_GET_VERSION  = SYSTEM_COMMAND_TYPE | 0b0001
} COMMAND_TYPE;

typedef struct
{
    COMMAND_TYPE Command;                  // command type, such as wheel command (up, down, stop), memory command (save, get, etc).
    portSHORT argv[COMMAND_ARGS_LIMIT];
    portCHAR argc;
} WheelCommand;

#endif /* SOURCE_INCLUDES_COMMANDS_H_ */
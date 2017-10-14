/*
 * Commands.h
 *
 *  Created on: 27 ρεπο. 2017 π.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_COMMANDS_H_
#define SOURCE_INCLUDES_COMMANDS_H_

#include "sys_common.h"
#include "string.h"
#include "FreeRTOS.h"
#include "Constants.h"

#define MAX_COMMAND_LEN 10
#define COMMANDS_LIMIT_COUNT 15
#define COMMAND_ARGS_LIMIT 5


// TODO: move to bitmask, but change getting of wheel using these number from the array(!!!)
typedef enum
{
    FL_WHEEL = 0,  // front left
    FR_WHEEL = 1,  // front right
    BL_WHEEL = 2,  // back left
    BR_WHEEL = 3,   // back right
    ALL_WHEELS = 4
} WHEEL;

typedef enum
{
    UNKNOWN_COMMAND = 0b00000000,

    WHEEL_COMMAND_TYPE = 0b10000000,
    CMD_WHEEL_UP   = WHEEL_COMMAND_TYPE | 0b00000001,
    CMD_WHEEL_DOWN = WHEEL_COMMAND_TYPE | 0b00000010,
    CMD_WHEEL_STOP = WHEEL_COMMAND_TYPE | 0b00000100,

    MEMORY_COMMAND_TYPE = 0b01000000,
    CMD_MEMORY_SAVE  = MEMORY_COMMAND_TYPE | 0b00000001,
    CMD_MEMORY_CLEAR = MEMORY_COMMAND_TYPE | 0b00000010,
    CMD_MEMORY_GET   = MEMORY_COMMAND_TYPE | 0b00000100
} COMMAND_TYPE;

typedef struct
{
    COMMAND_TYPE Command;                  // command type, such as wheel command (up, down, stop), memory command (save, get, etc).
    portSHORT argv[COMMAND_ARGS_LIMIT];
    portCHAR argc;
} WheelCommand;
typedef WheelCommand* CommandPtr;

/*typedef struct
{
    WHEEL wheelNumber;
    WheelCommand wheelCommand;
} Command;*/

#endif /* SOURCE_INCLUDES_COMMANDS_H_ */

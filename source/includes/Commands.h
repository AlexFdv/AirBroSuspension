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

#define MAX_COMMAND_LEN 10
#define COMMANDS_LIMIT_COUNT 15
#define COMMAND_ARGS_LIMIT 5

#define WHEELS_COUNT 4
typedef enum
{
    FL_WHEEL = 0,  // front left
    FR_WHEEL = 1,  // front right
    BL_WHEEL = 2,  // back left
    BR_WHEEL = 3   // back right
} WHEELS;

typedef enum
{
    WHEEL_COMMAND_TYPE = 0b10000000,
    WHEEL_UP   = WHEEL_COMMAND_TYPE | 0b00000001,
    WHEEL_DOWN = WHEEL_COMMAND_TYPE | 0b00000010,
    WHEEL_STOP = WHEEL_COMMAND_TYPE | 0b00000100,

    MEMORY_COMMAND_TYPE = 0b01000000,
    MEMORY_SAVE  = MEMORY_COMMAND_TYPE | 0b00000001,
    MEMORY_CLEAR = MEMORY_COMMAND_TYPE | 0b00000010,
    MEMORY_GET   = MEMORY_COMMAND_TYPE | 0b00000100
} COMMAND_TYPE;

typedef struct
{
    COMMAND_TYPE Command;                  // command type, such as wheel command (up, down, stop), memory command (save, get, etc).
    portCHAR argv[COMMAND_ARGS_LIMIT];
    portCHAR argc;
} Command;
typedef Command* CommandPtr;

#endif /* SOURCE_INCLUDES_COMMANDS_H_ */

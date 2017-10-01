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
#define MAX_REGISTERED_COMMANDS 15

typedef void (*commandFunctionPtr)();

typedef struct
{
    portCHAR commandName[MAX_COMMAND_LEN];
    portSHORT commandLen;
    commandFunctionPtr action;
} Command;

static Command registeredCommands[MAX_REGISTERED_COMMANDS] = {'\0'};
static uint16 uCurrentNumberOfCommands = 0;

Command* getCommand(const portCHAR receivedCommand[MAX_COMMAND_LEN]);
bool registerCommandByName(const portCHAR* commandName, commandFunctionPtr funcPtr);
bool registerCommand(const Command* cmd);

#endif /* SOURCE_INCLUDES_COMMANDS_H_ */

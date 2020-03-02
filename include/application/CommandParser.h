/*
 * Parser.h
 *
 *  Created on: 6 ����. 2018 �.
 *      Author: Alex
 */

#ifndef _APS_COMMANDPARSER_H_
#define _APS_COMMANDPARSER_H_

#include "CommandStructs.h"
#include "os_portmacro.h"

typedef bool (*commandHandler)(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc);

typedef struct
{
    COMMAND_TYPE cmdType;
    portCHAR* cmdValue;
    portSHORT cmdLen;
    commandHandler handler;
} CommandInfo;

Command parseCommand(const portCHAR command[MAX_COMMAND_LEN]);
void parseParams(const char* const strCmd, Command* const retCommand );

bool executeCommand(const Command* command);

#endif /* _APS_COMMANDPARSER_H_ */

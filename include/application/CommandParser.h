/*
 * Parser.h
 *
 *  Created on: 6 ����. 2018 �.
 *      Author: Alex
 */

#ifndef INCLUDE_APPLICATION_COMMANDPARSER_H_
#define INCLUDE_APPLICATION_COMMANDPARSER_H_

#include "CommandStructs.h"
#include "os_portmacro.h"

typedef struct
{
    COMMAND_TYPE cmdType;
    portCHAR* cmdValue;
    portSHORT cmdLen;
} CommandInfo;

Command parseCommand(const portCHAR command[MAX_COMMAND_LEN]);
void parseParams(const char* const strCmd, Command* const retCommand );

#endif /* INCLUDE_APPLICATION_COMMANDPARSER_H_ */

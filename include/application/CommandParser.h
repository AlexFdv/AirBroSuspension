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

typedef portSHORT (*commandHandler)(Command *cmd);

Command parseCommand(const portCHAR command[MAX_COMMAND_LEN]);
void parseParams(const char* const strCmd, Command* const retCommand );

bool executeCommand(Command* command);

bool registerCommandHandler(COMMAND_TYPE cmdType, commandHandler handler);

#endif /* _APS_COMMANDPARSER_H_ */

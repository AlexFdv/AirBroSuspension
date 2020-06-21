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
#include "Utils/List.h"

typedef portSHORT (*commandHandler)(Command *cmd);

Command parseCommand(const portCHAR command[MAX_COMMAND_LEN]);

int executeCommand(List *list, Command* command, bool use_mask);

void registerCommandHandler(List *list, COMMAND_TYPE cmdType, commandHandler handler);

#endif /* _APS_COMMANDPARSER_H_ */

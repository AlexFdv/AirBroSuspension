/*
 * Parser.h
 *
 *  Created on: 6 זמגע. 2018 נ.
 *      Author: Alex
 */

#ifndef INCLUDE_APPLICATION_COMMANDPARSER_H_
#define INCLUDE_APPLICATION_COMMANDPARSER_H_

#include "WheelCommandStructs.h"
#include "os_portmacro.h"

typedef struct
{
    COMMAND_TYPE cmdType;
    portCHAR* cmdValue;
    portSHORT cmdLen;
} CommandInfo;

WheelCommand parseCommand(const portCHAR command[MAX_COMMAND_LEN]);
void parseParams(const char* const strCmd, WheelCommand* const retCommand );

#endif /* INCLUDE_APPLICATION_COMMANDPARSER_H_ */

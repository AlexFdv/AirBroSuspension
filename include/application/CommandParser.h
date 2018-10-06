/*
 * Parser.h
 *
 *  Created on: 6 ����. 2018 �.
 *      Author: Alex
 */

#ifndef INCLUDE_APPLICATION_COMMANDPARSER_H_
#define INCLUDE_APPLICATION_COMMANDPARSER_H_

#include "WheelCommandStructs.h"
#include "FreeRTOS.h"

WheelCommand parseCommand(portCHAR command[MAX_COMMAND_LEN]);
void parseParams(char* strCmd, WheelCommand* const retCommand );

#endif /* INCLUDE_APPLICATION_COMMANDPARSER_H_ */

/*
 * CommandParser.c
 *
 *  Created on: 6 זמגע. 2018 נ.
 *      Author: Alex
 */

#include "CommandParser.h"
#include "string.h"
#include "stdlib.h"
#include "StringUtils.h"

#define ARRSIZE(x) (sizeof(x) / sizeof((x)[0]))

static const CommandInfo CommandsList[] =
{   {CMD_DIAGNOSTIC, "diag"},
    {CMD_WHEEL_UP,   "up"},
    {CMD_WHEEL_DOWN, "down"},
    {CMD_WHEEL_STOP, "stop"},
    {CMD_WHEEL_AUTO, "auto"},
    {CMD_LEVELS_SAVE, "lsave"},
    {CMD_LEVELS_GET,  "lget"},
    {CMD_LEVELS_SHOW, "lshow"},
    {CMD_GET_BATTERY, "bat"},
    {CMD_COMPRESSOR,  "compr"},
    {CMD_GET_VERSION, "ver"}
};


WheelCommand parseCommand(portCHAR command[MAX_COMMAND_LEN])
{
    WheelCommand parsedCommand =
    {
         UNKNOWN_COMMAND,
         {0},
         0
    };

    portSHORT i = 0;
    for (; i<ARRSIZE(CommandsList); ++i)
    {
        if (0 == strcmp(command, CommandsList[i].cmdValue))
        {
            parsedCommand.Command = CommandsList[i].cmdType;
            parseParams(command, &parsedCommand);
            break;
        }
    }

    return parsedCommand;
}


void parseParams(char* strCmd, WheelCommand* const retCommand )
{
    portCHAR* str = strchr(strCmd, ' ');
    while (str != NULL)
    {
        ++str;
        if (isDigits(str, ' '))
        {
            portSHORT param = atoi(str);
            retCommand->argv[retCommand->argc] = param;
            retCommand->argc++;

            str = strchr(str, ' ');
        }
        else
        {
            break;
        }

        if (retCommand->argc >= COMMAND_ARGS_LIMIT)
        {
            break;
        }
    }
}

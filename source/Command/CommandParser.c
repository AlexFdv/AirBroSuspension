/*
 * CommandParser.c
 *
 *  Created on: 6 זמגע. 2018 נ.
 *      Author: Alex
 */

#include "CommandParser.h"


WheelCommand parseCommand(portCHAR command[MAX_COMMAND_LEN])
{
    WheelCommand parsedCommand =
    {
         UNKNOWN_COMMAND,
         {0},
         0
    };


    if (0 == strncmp(command, "diag", 4))
    {
        parsedCommand.Command = CMD_DIAGNOSTIC;
    }
    else
    if (0 == strncmp(command, "up", 2))
    {
        parsedCommand.Command = CMD_WHEEL_UP;
    }
    else
    if (0 == strncmp(command, "down", 4))
    {
        parsedCommand.Command = CMD_WHEEL_DOWN;
    }
    else
    if (0 == strncmp(command, "stop", 4))
    {
        parsedCommand.Command = CMD_WHEEL_STOP;
    }
    else
    if (0 == strncmp(command, "auto", 4))
    {
        parsedCommand.Command = CMD_WHEEL_AUTO;
    }
    else
    if (0 == strncmp(command, "lsave", 5))
    {
        parsedCommand.Command = CMD_LEVELS_SAVE;
    }
    else
    if (0 == strncmp(command, "lget", 4))
    {
        parsedCommand.Command = CMD_LEVELS_GET;
    }
    else
    if (0 == strncmp(command, "lshow", 5))
    {
        parsedCommand.Command = CMD_LEVELS_SHOW;
    }
    else
    if (0 == strncmp(command, "bat", 3))
    {
        parsedCommand.Command = CMD_GET_BATTERY;
    }
    else
    if (0 == strncmp(command, "compr", 5))
    {
        parsedCommand.Command = CMD_COMPRESSOR;
    }
    else
    if (0 == strncmp(command, "ver", 3))
    {
        parsedCommand.Command = CMD_GET_VERSION;
    }

    parseParams(command, &parsedCommand);

    return parsedCommand;
}


void parseParams(char* strCmd, WheelCommand* const retCommand )
{
    char* str = strchr(strCmd, ' ');
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

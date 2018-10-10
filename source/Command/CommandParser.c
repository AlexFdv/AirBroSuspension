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

//
// max command size is MAX_COMMAND_LEN = 10 for now.
//
static const CommandInfo CommandsList[] =
{   {CMD_DIAGNOSTIC, "diag", 4},
    {CMD_WHEEL_UP,   "up", 2},
    {CMD_WHEEL_DOWN, "down", 4},
    {CMD_WHEEL_STOP, "stop", 4},
    {CMD_WHEEL_AUTO, "auto", 4},
    {CMD_LEVELS_SAVE_MAX, "lsavemax", 8},   // put it before lsave
    {CMD_LEVELS_SAVE_MIN, "lsavemin", 8},   // put it before lsave
    {CMD_LEVELS_SAVE, "lsave", 5},
    {CMD_LEVELS_GET_MAX, "lgetmax", 7},        // put it before lget
    {CMD_LEVELS_GET_MIN, "lgetmin", 7},        // put it before lget
    {CMD_LEVELS_GET,  "lget", 4},
    {CMD_LEVELS_SHOW, "lshow", 5},
    {CMD_MEM_CLEAR, "memclear", 8},
    {CMD_GET_BATTERY, "bat", 3},
    {CMD_COMPRESSOR,  "compr", 5},
    {CMD_GET_VERSION, "ver", 3}
};


WheelCommand parseCommand(const portCHAR command[MAX_COMMAND_LEN])
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
        if (0 == strncmp(command, CommandsList[i].cmdValue, CommandsList[i].cmdLen))
        {
            parsedCommand.Command = CommandsList[i].cmdType;
            parseParams(command, &parsedCommand);
            break;
        }
    }

    return parsedCommand;
}


void parseParams(const char* const strCmd, WheelCommand* const retCommand )
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

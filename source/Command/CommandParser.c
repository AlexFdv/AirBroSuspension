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
{
    {CMD_WHEEL_UP,   "up", 2},   // done
    {CMD_WHEEL_DOWN, "down", 4},    //done
    {CMD_WHEEL_STOP, "stop", 4},    //done
    {CMD_WHEEL_AUTO, "auto", 4},    //
    {CMD_LEVELS_SAVE_MAX, "lmaxsave", 8},   //done
    {CMD_LEVELS_SAVE_MIN, "lminsave", 8},   //done
    {CMD_LEVELS_SAVE, "lsave", 5},
    {CMD_LEVELS_GET_MAX, "lmaxget", 7}, //done
    {CMD_LEVELS_GET_MIN, "lminget", 7}, //done
    {CMD_LEVELS_GET,  "lget", 4},   // done
    {CMD_LEVELS_SHOW, "lshow", 5},  //done
    {CMD_MEM_CLEAR, "memclear", 8},
    {CMD_GET_BATTERY, "bat", 3},    //done
    {CMD_GET_COMPRESSOR_PRESSURE, "getcompr", 8},   //done
    {CMD_SET_COMPRESSOR_MAX_PRESSURE, "cmaxsave", 8},   //
    {CMD_SET_COMPRESSOR_MIN_PRESSURE, "cminsave", 8},   //
   // {CMD_GET_COMPRESSOR_MAX_PRESSURE, "cmaxget", 7},
   // {CMD_GET_COMPRESSOR_MIN_PRESSURE, "cminget", 7},
    {CMD_GET_VERSION, "ver", 3}
};


Command parseCommand(const portCHAR command[MAX_COMMAND_LEN])
{
    Command parsedCommand =
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
            parsedCommand.commandType = CommandsList[i].cmdType;
            parseParams(command, &parsedCommand);
            break;
        }
    }

    return parsedCommand;
}


void parseParams(const char* const strCmd, Command* const retCommand )
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

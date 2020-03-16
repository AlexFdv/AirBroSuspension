/*
 * CommandParser.c
 *
 *  Created on: 6 ����. 2018 �.
 *      Author: Alex
 */

#include "CommandParser.h"
#include "Tasks.h"
#include "string.h"
#include "stdlib.h"
#include "StringUtils.h"
#include "FEEController.h"
#include "Protocol.h"
#include "Config.h"
#include "../RtosWrapper/Rtos.h"

#define ARRSIZE(x)          (sizeof(x) / sizeof((x)[0]))
#define DELIMITER_CHAR      (':')

static portSHORT helpHandler(Command *cmd);





typedef struct
{
    COMMAND_TYPE cmdType;
    portCHAR* cmdValue;
    portSHORT cmdLen;
    commandHandler handler;
} CommandInfo;

/**
 * 1. Make the possibility to register a command handler.
 * 2. The registered handlers should be triggered when command arrives. Command structure should be passed here.
 * 3. Handlers should be in separate files. Communication with tasks should be via queues.
 */

//
// max command lengh is MAX_COMMAND_LEN
//
static CommandInfo commandsList[] =
{
    {CMD_WHEEL_UP,                      "up",           2, NULL},
    {CMD_WHEEL_DOWN,                    "down",         4, NULL},
    {CMD_WHEEL_STOP,                    "stop",         4, NULL},
    {CMD_WHEEL_AUTO,                    "auto",         4, NULL},
    {CMD_LEVELS_SAVE_MAX,               "lmaxsave",     8, NULL},
    {CMD_LEVELS_SAVE_MIN,               "lminsave",     8, NULL},
    {CMD_LEVELS_SAVE,                   "lsave",        5, NULL},
    {CMD_LEVELS_GET_MAX,                "lmaxget",      7, NULL},
    {CMD_LEVELS_GET_MIN,                "lminget",      7, NULL},
    {CMD_LEVELS_GET,                    "lget",         4, NULL},
    {CMD_LEVELS_SHOW,                   "lshow",        5, NULL},
    {CMD_MEM_CLEAR,                     "memclear",     8, NULL},
    {CMD_GET_BATTERY,                   "bat",          3, NULL},
    {CMD_GET_COMPRESSOR_PRESSURE,       "getcompr",     8, NULL},
    {CMD_SET_COMPRESSOR_MAX_PRESSURE,   "cmaxsave",     8, NULL},
    {CMD_SET_COMPRESSOR_MIN_PRESSURE,   "cminsave",     8, NULL},
    {CMD_GET_COMPRESSOR_MAX_PRESSURE,   "cmaxget",      7, NULL},
    {CMD_GET_COMPRESSOR_MIN_PRESSURE,   "cminget",      7, NULL},
    {CMD_GET_VERSION,                   "ver",          3, NULL},
    {CMD_HELP,                          "help",         4, helpHandler},

    /* End of list. Please keep UNKNOWN_COMMAND as the last one */
    {UNKNOWN_COMMAND,                   "",             0, NULL},
};

bool registerCommandHandler(COMMAND_TYPE cmdType, commandHandler handler)
{
    portSHORT i = 0;

    for(; i < ARRSIZE(commandsList); ++i)
    {
        if(cmdType == commandsList[i].cmdType)
        {
            commandsList[i].handler = handler;
            return true;
        }
    }

    return false;
}

bool processCommand(const portCHAR command[MAX_COMMAND_LEN])
{
    Command cmd = parseCommand(command);
    return executeCommand(&cmd);
}

Command parseCommand(const portCHAR command[MAX_COMMAND_LEN])
{
    Command parsedCommand =
    {
         UNKNOWN_COMMAND,
         {0},
         0
    };

    if (command[0] != '#')
        return parsedCommand;

    portSHORT i = 0;
    for (; i<ARRSIZE(commandsList); ++i)
    {
        // TODO: Compare until ':'
        if (0 == strncmp(command+1, commandsList[i].cmdValue, commandsList[i].cmdLen)) //@todo: use strlen instead of commandsList[i].cmdLen?
        {
            parsedCommand.commandType = commandsList[i].cmdType;
            parseParams(command, &parsedCommand);
            break;
        }
    }

    return parsedCommand;
}

void parseParams(const char* const strCmd, Command* const retCommand )
{
    portCHAR* str = strchr(strCmd, DELIMITER_CHAR);
    while (str != NULL)
    {
        ++str;
        if (isDigits(str, DELIMITER_CHAR))
        {
            portSHORT param = atoi(str);
            retCommand->argv[retCommand->argc] = param;
            retCommand->argc++;

            str = strchr(str, DELIMITER_CHAR);
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


bool executeCommand(Command* command)
{
    portSHORT i = 0;
    bool rv = false;

    for (; i < ARRSIZE(commandsList); ++i)
    {
        if (command->commandType == commandsList[i].cmdType)
        {
            /* Command was found, run it! */
            if (commandsList[i].handler != NULL)
            {
                rv = (*commandsList[i].handler)(command);
            }
            break;
        }
    }

    return rv;
}

/* ------------------------ Command Handlers ------------------------ */


static portSHORT helpHandler(Command *cmd)
{
    int i = 0;

    while (UNKNOWN_COMMAND != commandsList[i].cmdType)
    {
        printSuccessString(commandsList[i].cmdValue);
        i++;
    }
    printSuccess();

    return 0;
}


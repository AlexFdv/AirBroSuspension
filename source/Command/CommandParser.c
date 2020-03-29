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

// TODO: move to wrapper
#include "os_list.h"

#define ARRSIZE(x)          (sizeof(x) / sizeof((x)[0]))
#define DELIMITER_CHAR      (':')

static portSHORT helpHandler(Command *cmd);





typedef struct
{
    COMMAND_TYPE cmdType;
    portCHAR* cmdValue;
    portSHORT cmdLen;
} CommandInfo;

//
// max command lengh is MAX_COMMAND_LEN
//
static CommandInfo commandsList[] =
{
    {CMD_WHEEL_UP,                      "up",           2},
    {CMD_WHEEL_DOWN,                    "down",         4},
    {CMD_WHEEL_STOP,                    "stop",         4},
    {CMD_WHEEL_AUTO,                    "auto",         4},
    {CMD_LEVELS_SAVE_MAX,               "lmaxsave",     8},
    {CMD_LEVELS_SAVE_MIN,               "lminsave",     8},
    {CMD_LEVELS_SAVE,                   "lsave",        5},
    {CMD_LEVELS_GET_MAX,                "lmaxget",      7},
    {CMD_LEVELS_GET_MIN,                "lminget",      7},
    {CMD_LEVELS_GET,                    "lget",         4},
    {CMD_LEVELS_SHOW,                   "lshow",        5},
    {CMD_MEM_CLEAR,                     "memclear",     8},
    {CMD_GET_BATTERY,                   "bat",          3},
    {CMD_GET_COMPRESSOR_PRESSURE,       "getcompr",     8},
    {CMD_SET_COMPRESSOR_MAX_PRESSURE,   "cmaxsave",     8},
    {CMD_SET_COMPRESSOR_MIN_PRESSURE,   "cminsave",     8},
    {CMD_GET_COMPRESSOR_MAX_PRESSURE,   "cmaxget",      7},
    {CMD_GET_COMPRESSOR_MIN_PRESSURE,   "cminget",      7},
    {CMD_GET_VERSION,                   "ver",          3},
    {CMD_HELP,                          "help",         4},
};

typedef struct
{
    COMMAND_TYPE cmdType;
    commandHandler handler;
} CommandHandler;

static CommandHandler commandHandlersList[] =
{
    { WHEEL_COMMAND_TYPE, NULL},
    { MEMORY_COMMAND_TYPE, NULL },
    { ENV_COMMAND_TYPE, NULL },
    { UNKNOWN_COMMAND, NULL}
};


// =============================== os_heap.h pvPortMalloc
/*typedef struct
{
    CmdInfo *next;
    CmdInfo *subCommands;

    COMMAND_TYPE cmdType;
    portCHAR* cmdValue;
    portSHORT cmdLen;
    commandHandler handler;
} CmdInfo;


static List_t cmdList = {};

portSHORT regCommand()
{
    if (!listLIST_IS_INITIALISED(&cmdList))
    {
        vListInitialise(&cmdList);
    }

    return 0;
}*/

// ===============================
bool registerCommandHandler(COMMAND_TYPE cmdType, commandHandler handler)
{
    portSHORT i = 0;

    for(; i < ARRSIZE(commandsList); ++i)
    {
        if(cmdType == commandHandlersList[i].cmdType)
        {
            commandHandlersList[i].handler = handler;
            return true;
        }
    }

    return false;
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

    for (; i < ARRSIZE(commandHandlersList); ++i)
    {
        if ((command->commandType & commandHandlersList[i].cmdType) == commandHandlersList[i].cmdType)
        {
            /* Command was found, run it! */
            if (commandHandlersList[i].handler != NULL)
            {
                rv = (*commandHandlersList[i].handler)(command);
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


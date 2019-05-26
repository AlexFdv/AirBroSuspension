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
#include "Protocol.h"
#include "Config.h"

#define ARRSIZE(x)          (sizeof(x) / sizeof((x)[0]))
#define DELIMITER_CHAR      (':')


/* ------------------------ Command Handlers ------------------------ */
static bool getVersionHandler(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc);
static bool getBatVoltageHandler(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc);
static bool getComprPressureHandler(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc);
static bool helpHandler(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc);
/* ------------------------------------------------------------------ */


//
// max command size is MAX_COMMAND_LEN = 10 for now.
// Don't forget to increment COMMANDS_NUMBER during new command adding!!!
//
static const CommandInfo CommandsList[] =
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
    {CMD_GET_BATTERY,                   "bat",          3, getBatVoltageHandler},
    {CMD_GET_COMPRESSOR_PRESSURE,       "getcompr",     8, getComprPressureHandler},
    {CMD_SET_COMPRESSOR_MAX_PRESSURE,   "cmaxsave",     8, NULL},
    {CMD_SET_COMPRESSOR_MIN_PRESSURE,   "cminsave",     8, NULL},
    {CMD_GET_COMPRESSOR_MAX_PRESSURE,   "cmaxget",      7, NULL},
    {CMD_GET_COMPRESSOR_MIN_PRESSURE,   "cminget",      7, NULL},
    {CMD_GET_VERSION,                   "ver",          3, getVersionHandler},
    {CMD_HELP,                          "help",         4, helpHandler},

    /* End of list. Please keep UNKNOWN_COMMAND as the last one */
    {UNKNOWN_COMMAND,                   "",             0, NULL},
};


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
    for (; i<ARRSIZE(CommandsList); ++i)
    {
        // TODO: Compare until ':'
        if (0 == strncmp(command+1, CommandsList[i].cmdValue, CommandsList[i].cmdLen))
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


bool executeCommand(const Command* command)
{
    int i = 0;
    bool rv = false;

    while (UNKNOWN_COMMAND != CommandsList[i].cmdType)
    {
        if (command->commandType == CommandsList[i].cmdType)
        {
            /* Command was found, run it! */
            if (CommandsList[i].handler != NULL)
            {
                rv = (*CommandsList[i].handler)(command->argv, command->argc);
            }
            break;
        }
        i++;
    }

    return rv;
}

/* ------------------------ Command Handlers ------------------------ */
static bool getBatVoltageHandler(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc)
{
    (void) argv;
    (void) argc;

    bool rv = false;
    portLONG batteryVoltage = 0;

    rv = getBatteryVoltage(&batteryVoltage);
    if (rv)
    {
        printSuccessNumber(batteryVoltage);
    }
    else
    {
        printError(UndefinedErrorCode, "Cannot get battery value");
    }

    return rv;
}


static bool getVersionHandler(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc)
{
    (void) argv;
    (void) argc;

    printSuccessString(APP_VERSION);
    return true;
}


static bool getComprPressureHandler(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc)
{
    (void) argv;
    (void) argc;

    bool rv = false;
    AdcValue_t level;

    rv = getCompressorPressure(&level);
    if (rv)
    {
        printSuccessNumber(level);
    }
    else
    {
        printError(UndefinedErrorCode, "Cannot get compressor pressure");
    }

    return rv;
}


static bool helpHandler(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc)
{
    (void) argv;
    (void) argc;

    int i = 0;

    while (UNKNOWN_COMMAND != CommandsList[i].cmdType)
    {
        printSuccessString(CommandsList[i].cmdValue);
        i++;
    }
    printSuccess();

    return true;
}

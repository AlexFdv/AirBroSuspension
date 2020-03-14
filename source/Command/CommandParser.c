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


/* ------------------------ Command Handlers ------------------------ */
static bool getVersionHandler(Command *cmd);
static bool getBatVoltageHandler(Command *cmd);
static bool getComprPressureHandler(Command *cmd);
static bool helpHandler(Command *cmd);
static bool levelsGetHandler(Command *cmd);
static bool levelsSaveHandler(Command *cmd);
static bool levelsShowHandler(Command *cmd);
static bool levelsSaveMaxHandler(Command *cmd);
static bool levelsSaveMinHandler(Command *cmd);
static bool levelsGetMinHandler(Command *cmd);
static bool levelsGetMaxHandler(Command *cmd);
static bool memClearHandler(Command *cmd);
static bool getComprMaxPressureHandler(Command *cmd);
static bool getComprMinPressureHandler(Command *cmd);
static bool setComprMinPressureHandler(Command *cmd);
static bool setComprMaxPressureHandler(Command *cmd);

/* ------------------------------------------------------------------ */

typedef enum
{
    SettingMin,
    SettingMax
} SETTING_TYPE;

typedef bool (*commandHandler)(Command *cmd);

typedef struct
{
    COMMAND_TYPE cmdType;
    portCHAR* cmdValue;
    portSHORT cmdLen;
    commandHandler handler;
} CommandInfo;

/**
 * 1. Make the possibility to register a command handler.
 * 2. The registered handlers should be called when command arrives. Command structure should be passed here.
 * 3. Handlers should be in separate files. Communication with tasks should be via queues.
 */

//
// max command lengh is MAX_COMMAND_LEN
//
static const CommandInfo CommandsList[] =
{
    {CMD_WHEEL_UP,                      "up",           2, NULL},
    {CMD_WHEEL_DOWN,                    "down",         4, NULL},
    {CMD_WHEEL_STOP,                    "stop",         4, NULL},
    {CMD_WHEEL_AUTO,                    "auto",         4, NULL},
    {CMD_LEVELS_SAVE_MAX,               "lmaxsave",     8, levelsSaveMaxHandler},
    {CMD_LEVELS_SAVE_MIN,               "lminsave",     8, levelsSaveMinHandler},
    {CMD_LEVELS_SAVE,                   "lsave",        5, levelsSaveHandler},
    {CMD_LEVELS_GET_MAX,                "lmaxget",      7, levelsGetMaxHandler},
    {CMD_LEVELS_GET_MIN,                "lminget",      7, levelsGetMinHandler},
    {CMD_LEVELS_GET,                    "lget",         4, levelsGetHandler},
    {CMD_LEVELS_SHOW,                   "lshow",        5, levelsShowHandler},
    {CMD_MEM_CLEAR,                     "memclear",     8, memClearHandler},
    {CMD_GET_BATTERY,                   "bat",          3, getBatVoltageHandler},
    {CMD_GET_COMPRESSOR_PRESSURE,       "getcompr",     8, getComprPressureHandler},
    {CMD_SET_COMPRESSOR_MAX_PRESSURE,   "cmaxsave",     8, setComprMaxPressureHandler},
    {CMD_SET_COMPRESSOR_MIN_PRESSURE,   "cminsave",     8, setComprMinPressureHandler},
    {CMD_GET_COMPRESSOR_MAX_PRESSURE,   "cmaxget",      7, getComprMaxPressureHandler},
    {CMD_GET_COMPRESSOR_MIN_PRESSURE,   "cminget",      7, getComprMinPressureHandler},
    {CMD_GET_VERSION,                   "ver",          3, getVersionHandler},
    {CMD_HELP,                          "help",         4, helpHandler},

    /* End of list. Please keep UNKNOWN_COMMAND as the last one */
    {UNKNOWN_COMMAND,                   "",             0, NULL},
};

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
    for (; i<ARRSIZE(CommandsList); ++i)
    {
        // TODO: Compare until ':'
        if (0 == strncmp(command+1, CommandsList[i].cmdValue, CommandsList[i].cmdLen)) //@todo: use strlen instead of CommandsList[i].cmdLen?
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


bool executeCommand(Command* command)
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
                rv = (*CommandsList[i].handler)(command);
            }
            break;
        }
        i++;
    }

    return rv;
}

/* ------------------------ Command Handlers ------------------------ */
static bool getBatVoltageHandler(Command *cmd)
{
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


static bool getVersionHandler(Command *cmd)
{
    printSuccessString(APP_VERSION);
    return true;
}


static bool getComprPressureHandler(Command *cmd)
{
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


static bool helpHandler(Command *cmd)
{
    int i = 0;

    while (UNKNOWN_COMMAND != CommandsList[i].cmdType)
    {
        printSuccessString(CommandsList[i].cmdValue);
        i++;
    }
    printSuccess();

    return true;
}


static bool levelsGetHandler(Command *cmd)
{
    portSHORT levelNumber = (cmd->argc != 0) ? cmd->argv[0] : 0;
    if (levelNumber < LEVELS_COUNT && cmd->argc != 0)
    {
        printSuccessLevels(&(getCachedWheelLevels()[levelNumber]));
    }
    else
    {
        printError(WrongLevelSpecifiedErrorCode, "Wrong level number specified");
    }

    return true;
}


static bool levelsSaveHandler(Command *cmd)
{
    short levelNumber = (cmd->argc != 0) ? cmd->argv[0] : 0;
    if (levelNumber >= LEVELS_COUNT || cmd->argc == 0)
    {
        printError(WrongLevelSpecifiedErrorCode, "Wrong level number specified");
        return false;
    }

    bool useDummyValue = (cmd->argc == 2);
    LevelValues currLevels;

    if (useDummyValue)
    {
        short i;
        for (i = 0; i< WHEELS_COUNT;++i)
        {
            currLevels.wheels[i] = cmd->argv[1];
        }
    }
    else if (!getCurrentWheelsLevelsValues(&currLevels))
    {
        return false;
    }

    GLOBAL_SYNC_START;
        setCachedWheelLevel(levelNumber, currLevels);
        writeLevels((void*)getCachedWheelLevels());
    GLOBAL_SYNC_END;

    printSuccessLevels(&currLevels);

    return true;
}


static bool levelsShowHandler(Command *cmd)
{
    LevelValues currLevels;
    if (getCurrentWheelsLevelsValues(&currLevels))
    {
        printSuccessLevels(&currLevels);
    }

    return true;
}


static inline bool saveLevels(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc, SETTING_TYPE type)
{
    LevelValues currLevel;
    if (getCurrentWheelsLevelsValues(&currLevel))
    {
        GLOBAL_SYNC_START;
            uint8_t i;
            LevelValues *levelValues = (type == SettingMin) ? &(getSettings()->levels_values_min) : &(getSettings()->levels_values_max);
            for (i = 0; i < WHEELS_COUNT; ++i)
            {
                levelValues->wheels[i] = (argc==0)?currLevel.wheels[i]:argv[0];
            }
            writeSettings(getSettings());
        GLOBAL_SYNC_END;

        printSuccessLevels(levelValues);
    }

    return true;
}


static bool levelsSaveMinHandler(Command *cmd)
{
    return saveLevels(cmd->argv, cmd->argc, SettingMin);
}


static bool levelsSaveMaxHandler(Command *cmd)
{
    return saveLevels(cmd->argv, cmd->argc, SettingMax);
}


static bool levelsGetMinHandler(Command *cmd)
{
    printSuccessLevels(&(getSettings()->levels_values_min));

    return true;
}


static bool levelsGetMaxHandler(Command *cmd)
{
    printSuccessLevels(&(getSettings()->levels_values_max));

    return true;
}


static bool memClearHandler(Command *cmd)
{
    formatFEE();

    return true;
}


static bool getComprMaxPressureHandler(Command *cmd)
{
    printSuccessNumber(getSettings()->compressor_preasure_max);

    return true;
}


static bool getComprMinPressureHandler(Command *cmd)
{
    printSuccessNumber(getSettings()->compressor_preasure_min);

    return true;
}


inline static bool setComprPressure(const portSHORT argv[COMMAND_ARGS_LIMIT], portCHAR argc, SETTING_TYPE type)
{
    AdcValue_t pressure;
    uint16_t *value = (type == SettingMin) ? &getSettings()->compressor_preasure_min : &getSettings()->compressor_preasure_max;
    if (getCompressorPressure(&pressure))
    {
        GLOBAL_SYNC_START;
            *value = (argc==0) ? pressure : argv[0];
            writeSettings(getSettings());
        GLOBAL_SYNC_END;

        printSuccessNumber(*value);
    }

    return true;
}

static bool setComprMinPressureHandler(Command *cmd)
{
    return setComprPressure(cmd->argv, cmd->argc, SettingMin);
}


static bool setComprMaxPressureHandler(Command *cmd)
{
    return setComprPressure(cmd->argv, cmd->argc, SettingMax);
}

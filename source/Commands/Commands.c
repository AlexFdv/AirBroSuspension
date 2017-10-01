#include "Commands.h"

Command* getCommand(const portCHAR commandName[])
{
    portSHORT i = 0;
    for (i = 0; i < uCurrentNumberOfCommands; ++i)
    {
        if (0 == strcmp(commandName, registeredCommands[i].commandName))
        {
            return registeredCommands + i;
        }
    }

    return NULL;
}

bool registerCommandByName(const portCHAR* commandName, commandFunctionPtr funcPtr)
{
    Command cmd;
    memset(cmd.commandName, 0, MAX_COMMAND_LEN);
    strncpy(cmd.commandName, commandName, strlen(commandName));
    cmd.commandLen = strlen(commandName);
    cmd.action = funcPtr;

    return registerCommand(&cmd);
}

bool registerCommand(const Command* cmd)
{
    if (uCurrentNumberOfCommands >= MAX_REGISTERED_COMMANDS)
    {
        return false;
    }

    memcpy(&registeredCommands[uCurrentNumberOfCommands], cmd, sizeof(Command));
    ++uCurrentNumberOfCommands;
    return true;
}

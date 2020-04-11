/*
 * util.c
 *
 *  Created on: Apr 11, 2020
 *      Author: Alex
 */
#include <Util.h>
#include "Protocol.h"

char *err2str(int code)
{
    switch (code)
    {
    case WrongLevelSpecifiedErrorCode:
        return "Wrong level number specified";
    case UndefinedErrorCode:
        return "Undefined error";
    case UnknownCommandErrorCode:
        return "Unknown command received";
    case WrongWheelSpecifiedErrorCode:
        return "Wrong wheel number specified";
    case QueueReadTimeoutErrorCode:
        return "Timeout at value reading from the queue";
    case QueueWheelLevelReadTimeoutErrorCode:
            return "Timeout at wheel level value reading from the queue";
    case MemoryQueueErrorCode:
        return "Timeout in memory queue";
    case CommandsQueueErrorCode:
        return "Could not receive a value from the queue.";
    }

    return "Unknown error occurred";
}

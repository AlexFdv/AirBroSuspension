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
    case QueueWheelLevelReadTimeoutErrorCode:
        return "Timeout at wheel level value reading from the queue";
    case QueueCompressorReadTimeoutErrorCode:
        return "Timeout at compressor value reading from the queue";
    case QueueBatteryReadTimeoutErrorCode:
        return "Timeout at battery value reading from the queue";
    case QueueAdcAverageReadTimeoutErrorCode:
        return "Timeout at ADC average value reading from the queue";
    case MemorySendQueueErrorCode:
        return "Timeout in sending to the memory queue";
    case MemoryReceiveQueueErrorCode:
        return "Timeout in receiving from the memory queue";
    case CommandsQueueErrorCode:
        return "Could not receive a value from the queue.";
    case WrongCommandParams:
        return "Wrong number of parameters";
    }

    return "Unknown error occurred";
}

/*
 * Protocol.h
 *
 *  Created on: May 26, 2019
 *      Author: oleg
 */

#ifndef _APS_PROTOCOL_H_
#define _APS_PROTOCOL_H_

#include <stddef.h>

#include "Levels.h"

enum error_codes
{
    UndefinedErrorCode = 0,
    UnknownCommandErrorCode = 1,
    WrongWheelSpecifiedErrorCode = 2,
    WrongLevelSpecifiedErrorCode = 3,
    QueueReadTimeoutErrorCode = 4,
    MemoryQueueErrorCode = 5,
    CommandsQueueErrorCode = 6
};

bool protocol_init(void);

void printError(int code, const char* text);

void printSuccess(void);

void printSuccessString(const char* text);

void printSuccessNumber(long number);

void printSuccessLevels(const LevelValues* const levels);

void sendDiagnosticData(const void *data, size_t len);

#endif /* _APS_PROTOCOL_H_ */

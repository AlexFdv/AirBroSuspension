/*
 * Protocol.h
 *
 *  Created on: May 26, 2019
 *      Author: oleg
 */

#ifndef INCLUDE_APPLICATION_PROTOCOL_H_
#define INCLUDE_APPLICATION_PROTOCOL_H_

#include "Levels.h"

void printError(int code, const char* text);

void printSuccess(void);

void printSuccessString(const char* text);

void printSuccessNumber(long number);

void printSuccessLevels(const LevelValues* const levels);

#endif /* INCLUDE_APPLICATION_PROTOCOL_H_ */

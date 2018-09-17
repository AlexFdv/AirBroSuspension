/*
 * SerialController.h
 *
 *  Created on: 18 ���. 2017 �.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_SERIALCONTROLLER_H_
#define SOURCE_INCLUDES_SERIALCONTROLLER_H_

#include "os_portmacro.h"

typedef void (*Callback)(uint8* data, short length);

void initializeSci(Callback dataCallback);
void sciDisplayData(const portCHAR *text, portSHORT length);
void sciDisplayDataLin(const portCHAR *text, short length);
void sciSendData(const uint8* data, portSHORT length);


void printText(const char* text);
void printNumber(const portLONG number);
void printText_ex(const char* text, short maxLen);

void printTextLin(const char* text);
void printNumberLin(const portLONG number);
void printTextLin_ex(const char* text, short maxLen);

#endif /* SOURCE_INCLUDES_SERIALCONTROLLER_H_ */

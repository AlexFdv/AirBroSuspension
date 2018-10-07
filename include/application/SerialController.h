/*
 * SerialController.h
 *
 *  Created on: 18 ���. 2017 �.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_SERIALCONTROLLER_H_
#define SOURCE_INCLUDES_SERIALCONTROLLER_H_

#include "os_portmacro.h"

typedef void (*Callback)(uint8* data, portSHORT length);

void initializeSci(Callback dataCallback);
void sciDisplayData(const portCHAR *text, portSHORT length);
void sciDisplayDataLin(const portCHAR *text, portSHORT length);
void sciSendData(const uint8* data, portSHORT length);

inline void printTextLin_ex(const portCHAR* text, const portSHORT maxLen)
{
    sciDisplayDataLin(text, maxLen);
}

inline void printText_ex(const portCHAR* text, const portSHORT maxLen)
{
    sciDisplayData(text, maxLen);
}

inline void printText(const portCHAR* text)
{
    printText_ex(text, strlen(text));

    // duplicate to debug out
    printTextLin_ex(text, strlen(text));
}


inline void printTextLin(const portCHAR* text)
{
    printTextLin_ex(text, strlen(text));
}

inline void printNumber(const portLONG number)
{
    char buff[10] = {'\0'};
    ltoa(number, buff);
    printText(buff);
}

inline void printNumberLin(const portLONG number)
{
    char buff[10] = {'\0'};
    ltoa(number, buff);
    printTextLin(buff);
}


#endif /* SOURCE_INCLUDES_SERIALCONTROLLER_H_ */

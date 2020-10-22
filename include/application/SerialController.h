/*
 * SerialController.h
 *
 *  Created on: 18 ���. 2017 �.
 *      Author: Alex
 */

#ifndef _APS_SERIALCONTROLLER_H_
#define _APS_SERIALCONTROLLER_H_

#include <string.h>
#include <stdlib.h>

#include "os_portmacro.h"

typedef void (*Callback)(uint8* data, portSHORT length);

void initializeSci(Callback dataCallback);
void sciDisplayData(const portCHAR *text, portSHORT length);
void sciDisplayDataLin(const portCHAR *text, portSHORT length);
void sciSendDataLin(const uint8* data, portSHORT length);

inline void printText(const portCHAR* text)
{
    sciDisplayData(text, strlen(text));
}


inline void printTextLin(const portCHAR* text)
{
    sciDisplayDataLin(text, strlen(text));
}

inline void printNumberLin(const portLONG number)
{
    char buff[10] = {'\0'};
    ltoa(number, buff, 10);
    printTextLin(buff);
}

inline void printNumber(const portLONG number)
{
    char buff[10] = {'\0'};
    ltoa(number, buff, 10);
    printText(buff);
}




#endif /* _APS_SERIALCONTROLLER_H_ */

/*
 * SerialController.c
 *
 *  Created on: 18 лип. 2017 р.
 *      Author: Alex
 */
#include "sci.h"
#include "os_portmacro.h"
#include "SerialController.h"

void initializeSci()
{
    sciInit();
}

void sciDisplayData(const portCHAR *text, portSHORT length)
{
    sciBASE_t *sciReg = scilinREG;
    while (length--)
    {
        uint8 chr = (uint8)(*(text++));
        while ((sciReg->FLR & 0x4) == 4)
            ; /* wait until busy */
        sciSendByte(sciReg, chr); /* send out text */
    };
}

void sciReceiveData(portCHAR *receivedText, portSHORT *receivedLength, portSHORT maxLength)
{
    const uint32 STOP_CHAR = 0x0D;

    portSHORT receivedSize = 0;
    uint32 ch = 0x00;
    do
    {
        ch = sciReceiveByte(scilinREG);
        if (ch == STOP_CHAR)
            break;
        receivedText[receivedSize] = (portCHAR)ch;
        ++receivedSize;
    } while (receivedSize < maxLength);

    *receivedLength = receivedSize;
}

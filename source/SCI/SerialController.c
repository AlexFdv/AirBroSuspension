/*
 * SerialController.c
 *
 *  Created on: 18 лип. 2017 р.
 *      Author: Alex
 */
#include "sci.h"
#include "SerialController.h"

void initializeSci()
{
    sciInit();
}

void sciDisplayData(const char *text, short length)
{
    sciBASE_t *sciReg = scilinREG;
    while (length--)
    {
        uint32 chr = (uint32)(*(text++));
        while ((sciReg->FLR & 0x4) == 4)
            ; /* wait until busy */
        sciSendByte(sciReg, chr); /* send out text */
    };
}

void sciReceiveData(char *receivedtext, short *receivedLength, short maxLength)
{
    const uint32 STOP_CHAR = 0x0D;

    short receivedSize = 0;
    uint32 ch = 0x00;
    do
    {
        ch = sciReceiveByte(scilinREG);
        if (ch == STOP_CHAR)
            break;
        receivedtext[receivedSize] = (char)ch;
        ++receivedSize;
    } while (receivedSize < maxLength);

    *receivedLength = receivedSize;
}

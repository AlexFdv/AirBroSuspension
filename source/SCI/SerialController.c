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

void sciDisplayText(uint32 *text, uint32 length)
{
    sciBASE_t *sciReg = scilinREG;
    while (length--)
    {
        uint32 chr = *(text++);
        while ((sciReg->FLR & 0x4) == 4)
            ; /* wait until busy */
        sciSendByte(sciReg, chr); /* send out text */
    };
}

bool sciReceiveText(uint32 *outtext, uint32 maxLength, uint32 *receivedLength)
{
    uint32 receivedSize = 0;
    if(sciIsRxReady(scilinREG))
    {
        uint32 ch = 0x00;
        while (receivedSize < maxLength)
        {
            ch = sciReceiveByte(scilinREG);
            if (ch == 0x0D)
                break;
            outtext[receivedSize] = ch;
            ++receivedSize;

        }

        *receivedLength = receivedSize;
        return true;
    }
    else
    {
        return false;
    }
}

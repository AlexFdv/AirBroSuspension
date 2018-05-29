/*
 * SerialController.c
 *
 *  Created on: 18 ���. 2017 �.
 *      Author: Alex
 */
#include "sci.h"
#include "os_portmacro.h"
#include "SerialController.h"
#include "ConstantsCommon.h"
#include "string.h"

#define SCILIN_REG scilinREG    // output to debug terminal
#define SCI_REG sciREG          // output via bluetooth

typedef void (*Callback)(portCHAR* data, portSHORT length);
Callback dataReceivedCallback;

portSHORT receivedLen = 0;
portCHAR receivedCommand[MAX_COMMAND_LEN] = {'\0'};
portCHAR receivedByte;

#define STOP_CHAR 0x0D

void initializeSci(Callback dataCallback)
{
    sciInit();
    dataReceivedCallback = dataCallback;

    memset(receivedCommand, 0, MAX_COMMAND_LEN);

    sciReceive(SCI_REG, 1, &receivedByte);
}

void sciNotification(sciBASE_t *sci, uint32 flags)
{
    do
    {
        if (receivedByte == STOP_CHAR || receivedLen >= MAX_COMMAND_LEN)
        {
            if (dataReceivedCallback != NULL_PTR)
                dataReceivedCallback(receivedCommand, receivedLen);

            memset(receivedCommand, 0, MAX_COMMAND_LEN);
            receivedLen = 0;

            break;
        }

        receivedCommand[receivedLen++] = receivedByte;
    } while (0);

    sciReceive(SCI_REG, 1, &receivedByte);
}

void sciDisplayDataEx(sciBASE_t *sciReg, const portCHAR *text, portSHORT length)
{
    while (length--)
    {
        uint8 chr = (uint8)(*(text++));
        while ((sciReg->FLR & 0x4) == 4)
            ; /* wait until busy */
        sciSendByte(sciReg, chr); /* send out text */
    };
}

void sciDisplayData(const portCHAR *text, portSHORT length)
{
    sciDisplayDataEx(SCI_REG, text, length);

    // duplicate the output to terminal
    //sciDisplayDataEx(SCILIN_REG, text, length);
}

void sciReceiveDataEx(sciBASE_t *sciReg, portCHAR *receivedText, portSHORT *receivedLength, portSHORT maxLength)
{
    /*const uint32 STOP_CHAR = 0x0D;

    portSHORT receivedSize = 0;
    uint32 ch = 0x00;
    do
    {
        ch = sciReceiveByte(sciReg);
        if (ch == STOP_CHAR)
            break;
        receivedText[receivedSize] = (portCHAR)ch;
        ++receivedSize;
    } while (receivedSize < maxLength);

    *receivedLength = receivedSize;*/
}

void sciReceiveData(portCHAR *receivedText, portSHORT *receivedLength, portSHORT maxLength)
{
    sciReceiveDataEx(SCI_REG, receivedText, receivedLength, maxLength);
}

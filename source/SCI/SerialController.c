/*
 * SerialController.c
 *
 *  Created on: 18 ���. 2017 �.
 *      Author: Alex
 */

#include <stdlib.h>
#include "sci.h"
#include "os_portmacro.h"
#include "string.h"
#include <application/ConstantsCommon.h>
#include <application/SerialController.h>


//#define SCILIN_REG scilinREG    // output to debug terminal
//#define SCI_REG sciREG          // output via bluetooth
#define SCILIN_REG sciREG    // output to debug terminal
#define SCI_REG scilinREG          // output via bluetooth

Callback dataReceivedCallback;

short receivedLen = 0;
uint8 receivedCommand[MAX_COMMAND_LEN] = {'\0'};
uint8 receivedByte;

#define STOP_CHAR 0x0A //0x0D = enter, 0x0A = \n

void initializeSci(Callback dataCallback)
{
    sciInit();
    dataReceivedCallback = dataCallback;

    memset(receivedCommand, 0, MAX_COMMAND_LEN);

    // obligatory calls to finish initialization
    sciReceive(SCI_REG, 1, &receivedByte);
    sciReceive(SCILIN_REG, 1, &receivedByte);
}

void sciNotification(sciBASE_t *sci, uint32 flags)
{
    receivedLen++;
    if (receivedByte == STOP_CHAR || receivedLen >= MAX_COMMAND_LEN - 1)
    {
        // callback to inform that reading is finished
        if (dataReceivedCallback != NULL_PTR)
            dataReceivedCallback(receivedCommand, receivedLen);

        // reset buffer before the next calls
        memset(receivedCommand, 0, MAX_COMMAND_LEN);
        receivedLen = 0;
    }
    else
        receivedCommand[receivedLen - 1] = receivedByte;

    sciReceive(sci, 1, &receivedByte);
}

static void sciDisplayDataEx(sciBASE_t *sciReg, const portCHAR *text, portSHORT length)
{
    while (length--)
    {
        uint8 chr = (uint8)(*(text++));
        while ((sciReg->FLR & 0x4) == 4)
            ; /* wait until busy */
        sciSendByte(sciReg, chr); /* send out text */
    };
}

void sciSendDataLin(const uint8* data, portSHORT length)
{
    while (length--)
    {
        while ((sciREG->FLR & 0x4) == 4)
            ; /* wait until busy */
        sciSendByte(SCILIN_REG, *(data++)); /* send out text */
    };
}

void sciDisplayData(const portCHAR *text, portSHORT length)
{
    sciDisplayDataEx(SCI_REG, text, length);
}

void sciDisplayDataLin(const portCHAR *text, portSHORT length)
{
    // output to terminal
    sciDisplayDataEx(SCILIN_REG, text, length);
}




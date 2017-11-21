/*
 * FEEController.c
 *
 *  Created on: 15 זמגע. 2017 נ.
 *      Author: Alex
 */

#include "FEEController.h"

#include "ti_fee.h"

void delay(void)
{
    unsigned int dummycnt=0x0000FFU;
    do
    {
        dummycnt--;
    } while(dummycnt > 0);
}

void waitForExecution()
{
    do
    {
        TI_Fee_MainFunction();
        delay();
    } while(TI_Fee_GetStatus(0) != IDLE);
}

void initializeFEE()
{
    TI_Fee_Init();

    waitForExecution();
}

void formatFEE()
{
    TI_Fee_Format(0xA5A5A5A5U);

    waitForExecution();
}

// try use async functions
void writeSyncFEE(const void* value, unsigned int len)
{
    const unsigned int blockNumber = 1;
    uint8 SpecialRamBlock[50] = {0};

    memcpy(&SpecialRamBlock[0], value, len);

    TI_Fee_WriteSync(blockNumber, &SpecialRamBlock[0]);
}

void readSyncFEE(void* value, unsigned int len)
{
    const unsigned int blockNumber = 1;

    // Fee_BlockConfiguration[0].FeeBlockSize
    unsigned char read_data[50] = {0};
    unsigned int blockOffset = 0;

    TI_Fee_ReadSync(blockNumber, blockOffset, &read_data[0], len);

    memcpy(value, &read_data[0], len);

    return 0;
}

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

void writeSyncFEE(unsigned int blockNumber, unsigned char value)
{
    TI_Fee_WriteSync(blockNumber, &value);
}

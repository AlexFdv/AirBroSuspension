/*
 * FEEController.c
 *
 *  Created on: 15 ����. 2017 �.
 *      Author: Alex
 */

#include <application/FEEController.h>
#include "ti_fee.h"

void static delay(void)
{
    unsigned int dummycnt=0x0000FFU;
    do
    {
        dummycnt--;
    } while(dummycnt > 0);
}

void static waitForExecution()
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

void writeLevels(void* levels)
{
    writeSyncFEE(LEVELS_BLOCK_NUMBER, (uint8*)levels);
}

void readLevels(void* levels)
{
    readSyncFEE(LEVELS_BLOCK_NUMBER, levels, LEVELS_BLOCK_SIZE);
}

void writeSettings(void* settings)
{
    writeSyncFEE(SETTINGS_BLOCK_NUMBER, settings);
}

void readSettings(void* settings)
{
    readSyncFEE(SETTINGS_BLOCK_NUMBER, settings, SETTINGS_BLOCK_SIZE);
}

// TODO: try use async functions
void writeSyncFEE(const uint16 blockNumber, void* value)
{
    TI_Fee_WriteSync(blockNumber, value);
}

void readSyncFEE(const uint16 blockNumber, void* value, const unsigned int len)
{
    unsigned int blockOffset = 0;
    TI_Fee_ReadSync(blockNumber, blockOffset, value, len);
}

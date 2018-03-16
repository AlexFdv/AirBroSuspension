/*
 * FEEController.h
 *
 *  Created on: 15 ����. 2017 �.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_FEECONTROLLER_H_
#define SOURCE_INCLUDES_FEECONTROLLER_H_

#include "ti_fee.h"

#define LEVELS_BLOCK_SIZE Fee_BlockConfiguration[0].FeeBlockSize
#define LEVELS_BLOCK_NUMBER Fee_BlockConfiguration[0].FeeBlockNumber

#define SETTINGS_BLOCK_SIZE Fee_BlockConfiguration[1].FeeBlockSize
#define SETTINGS_BLOCK_NUMBER Fee_BlockConfiguration[1].FeeBlockNumber

void initializeFEE();
void formatFEE();
void writeLevels(void* levels);
void readLevels(void* levels);
void writeSyncFEE(const uint16 blockNumber, uint8 * value);
void readSyncFEE(const uint16 blockNumber, void* value, unsigned int len);

#endif /* SOURCE_INCLUDES_FEECONTROLLER_H_ */

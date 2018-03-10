/*
 * FEEController.h
 *
 *  Created on: 15 זמגע. 2017 נ.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_FEECONTROLLER_H_
#define SOURCE_INCLUDES_FEECONTROLLER_H_

#include "ti_fee.h"

#define LEVELS_BLOCK_SIZE Fee_BlockConfiguration[0].FeeBlockSize
#define LEVELS_BLOCK_NUMBER Fee_BlockConfiguration[0].FeeBlockNumber

void initializeFEE();
void formatFEE();
void writeLevels(void* levels);
void readLevels(void* levels);
void writeSyncFEE(const unsigned int blockNumber, uint8 * value);
void readSyncFEE(const unsigned int blockNumber, void* value, unsigned int len);

#endif /* SOURCE_INCLUDES_FEECONTROLLER_H_ */

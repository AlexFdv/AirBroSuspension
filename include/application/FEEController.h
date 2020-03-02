/*
 * FEEController.h
 *
 *  Created on: 15 זמגע. 2017 נ.
 *      Author: Alex
 */

#ifndef _APS_FEECONTROLLER_H_
#define _APS_FEECONTROLLER_H_

#include "ti_fee.h"

#define LEVELS_BLOCK_SIZE Fee_BlockConfiguration[0].FeeBlockSize
#define LEVELS_BLOCK_NUMBER Fee_BlockConfiguration[0].FeeBlockNumber

#define SETTINGS_BLOCK_SIZE Fee_BlockConfiguration[1].FeeBlockSize
#define SETTINGS_BLOCK_NUMBER Fee_BlockConfiguration[1].FeeBlockNumber

void initializeFEE();
void formatFEE();
void writeLevels(void* levels);
void readLevels(void* levels);
void readSettings(void* settings);
void writeSettings(void* settings);
void writeSyncFEE(const uint16 blockNumber, void * value);
void readSyncFEE(const uint16 blockNumber, void* value, unsigned int len);

#endif /* _APS_FEECONTROLLER_H_ */

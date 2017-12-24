/*
 * FEEController.h
 *
 *  Created on: 15 זמגע. 2017 נ.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_FEECONTROLLER_H_
#define SOURCE_INCLUDES_FEECONTROLLER_H_

void initializeFEE();
void formatFEE();
void writeSyncFEE(const unsigned int blockNumber, const void* value);
void readSyncFEE(const unsigned int blockNumber, void* value, unsigned int len);

#endif /* SOURCE_INCLUDES_FEECONTROLLER_H_ */

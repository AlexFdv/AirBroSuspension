/*
 * FEEController.h
 *
 *  Created on: 15 ����. 2017 �.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_FEECONTROLLER_H_
#define SOURCE_INCLUDES_FEECONTROLLER_H_

void initializeFEE();
void formatFEE();
void writeSyncFEE(const void* value, unsigned int len);
void readSyncFEE(void* value, unsigned int len);

#endif /* SOURCE_INCLUDES_FEECONTROLLER_H_ */

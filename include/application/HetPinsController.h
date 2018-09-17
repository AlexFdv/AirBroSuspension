/*
 * HetPinsController.h
 *
 *  Created on: 12 ���. 2017 �.
 *      Author: Alex Fadeev
 */

#ifndef SOURCE_HETPINS_INCLUDES_HETPINSCONTROLLER_H_
#define SOURCE_HETPINS_INCLUDES_HETPINSCONTROLLER_H_

void initializeHetPins();
void openPin(uint32 pin);
void togglePin(uint32 pin);
void closePin(uint32 pin);
uint32 getPin(uint32 pin);

#endif /* SOURCE_HETPINS_INCLUDES_HETPINSCONTROLLER_H_ */

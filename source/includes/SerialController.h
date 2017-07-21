/*
 * SerialController.h
 *
 *  Created on: 18 бер. 2017 р.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_SERIALCONTROLLER_H_
#define SOURCE_INCLUDES_SERIALCONTROLLER_H_

void initializeSci();
void sciDisplayText(uint32 *text, uint32 length);
bool sciReceiveText(uint32 *outtext, uint32 maxLength, uint32 *receivedLength);

#endif /* SOURCE_INCLUDES_SERIALCONTROLLER_H_ */

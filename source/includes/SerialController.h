/*
 * SerialController.h
 *
 *  Created on: 18 ���. 2017 �.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_SERIALCONTROLLER_H_
#define SOURCE_INCLUDES_SERIALCONTROLLER_H_

void initializeSci();
void sciDisplayData(const char *text, short length);
void sciReceiveData(char *outtext, short *receivedLength, short maxLength);

#endif /* SOURCE_INCLUDES_SERIALCONTROLLER_H_ */

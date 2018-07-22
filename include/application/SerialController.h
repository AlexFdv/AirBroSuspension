/*
 * SerialController.h
 *
 *  Created on: 18 бер. 2017 р.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_SERIALCONTROLLER_H_
#define SOURCE_INCLUDES_SERIALCONTROLLER_H_

typedef void (*Callback)(uint8* data, short length);

void initializeSci(Callback dataCallback);
void sciDisplayDataLin(const char *text, short length);

#endif /* SOURCE_INCLUDES_SERIALCONTROLLER_H_ */

/*
 * ADCController.h
 *
 *  Created on: 7 ñ³÷. 2018 ð.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_ADCCONTROLLER_H_
#define SOURCE_INCLUDES_ADCCONTROLLER_H_

#include "adc.h"
#include "Constants.h"

void initializeADC();
void startADCConversion(short wheelNumber);
void stopADCConversion(short wheelNumber);
void getADCValues(adcData_t* adcData);

#endif /* SOURCE_INCLUDES_ADCCONTROLLER_H_ */

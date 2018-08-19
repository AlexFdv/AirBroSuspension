/*
 * ADCController.h
 *
 *  Created on: 7 ��. 2018 �.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_ADCCONTROLLER_H_
#define SOURCE_INCLUDES_ADCCONTROLLER_H_

#include "../application/ConstantsCommon.h"
#include "adc.h"

/*
 * PINS 8, 11, 13, 14 approved
 * */

void initializeADC();
void getADCValues(adcData_t* adcData);

#endif /* SOURCE_INCLUDES_ADCCONTROLLER_H_ */

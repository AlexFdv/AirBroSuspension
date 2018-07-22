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

void initializeADC();
void getADCValues(adcData_t* adcData);

#endif /* SOURCE_INCLUDES_ADCCONTROLLER_H_ */

/*
 * ADCController.c
 *
 *  Created on: 7 ��. 2018 �.
 *      Author: Alex
 */


#include <application/ADCController.h>
#include "adc.h"

#include "FreeRTOS.h"
#include "os_semphr.h"

void initializeADC()
{
    adcInit();
}

void getADCValues(adcData_t* adcData)
{
    // it is not "continue" conversion
    adcStartConversion(adcREG1, adcGROUP1);
    while ((adcIsConversionComplete(adcREG1, adcGROUP1)) == 0)
                    ;
    adcGetData(adcREG1, adcGROUP1, adcData);
}

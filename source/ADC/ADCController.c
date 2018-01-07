/*
 * ADCController.c
 *
 *  Created on: 7 ñ³÷. 2018 ð.
 *      Author: Alex
 */


#include "ADCController.h"
#include "adc.h"

void initializeADC()
{
    adcInit();

    //adcStartConversion(adcREG1, adcGROUP1);
}


void getADCValues(adcData_t* adcData)
{
    adcStartConversion(adcREG1, adcGROUP1);
    while ((adcIsConversionComplete(adcREG1, adcGROUP1)) == 0)
                    ;
    adcGetData(adcREG1, adcGROUP1, adcData);
}

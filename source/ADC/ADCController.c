/*
 * ADCController.c
 *
 *  Created on: 7 ��. 2018 �.
 *      Author: Alex
 */


#include <application/ADCController.h>
#include <application/ConstantsCommon.h>
#include "adc.h"

#include "FreeRTOS.h"
#include "os_portable.h"


static const _idx_value mapper[ADC_FIFO_SIZE] = {
     {FL_WHEEL_IDX, FL_WHEEL_ADC_PIN},
     {FR_WHEEL_IDX, FR_WHEEL_ADC_PIN},
     {BL_WHEEL_IDX, BL_WHEEL_ADC_PIN},
     {BR_WHEEL_IDX, BR_WHEEL_ADC_PIN},
     {COMPRESSOR_IDX, COMPRESSOR_ADC_PIN},
     {BATTERY_IDX, BATTERY_ADC_PIN},
};

static adcData_t* adcDataBuffer;

void initializeADC()
{
    adcInit();

    adcDataBuffer = (adcData_t*)pvPortMalloc(ADC_FIFO_SIZE * sizeof(adcData_t));
}

void getADCDataValues(AdcDataValues* adcData)
{
    adcStartConversion(adcREG1, adcGROUP1);
    while ((adcIsConversionComplete(adcREG1, adcGROUP1)) == 0)
        ;

    adcGetData(adcREG1, adcGROUP1, adcDataBuffer);

    // map received values
    for (short i = 0; i < ADC_FIFO_SIZE; ++i)
    {
        // check pins in mapper array
        for (short j = 0; j < ADC_FIFO_SIZE; ++j)
        {
            if (adcDataBuffer[i].id == mapper[j].adc_pin)
            {
                short dataIdx = mapper[j].data_idx;
                adcData->values[dataIdx] = adcDataBuffer[i].value;
            }
        }
    }
}

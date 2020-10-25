/*
 * ADCController.c
 *
 *  Created on: 7 ��. 2018 �.
 *      Author: Alex
 */


#include <application/ADCController.h>
#include <application/ConstantsCommon.h>

#include "FreeRTOS.h"
#include "adc.h"

static const _idx_value mapper[ADC_FIFO_SIZE] = {
     {FL_WHEEL_IDX, FL_WHEEL_ADC_PIN},
     {FR_WHEEL_IDX, FR_WHEEL_ADC_PIN},
     {BL_WHEEL_IDX, BL_WHEEL_ADC_PIN},
     {BR_WHEEL_IDX, BR_WHEEL_ADC_PIN},
     {COMPRESSOR_IDX, COMPRESSOR_ADC_PIN},
     {BATTERY_IDX, BATTERY_ADC_PIN},
};

static adcData_t* adcRawDataBuffer;

void initializeADC()
{
    adcInit();

    adcRawDataBuffer = (adcData_t*)pvPortMalloc(ADC_FIFO_SIZE * sizeof(adcData_t));
}

void getADCDataValues(AdcDataValues* adcData)
{
    adcStartConversion(adcREG1, adcGROUP1);
    while ((adcIsConversionComplete(adcREG1, adcGROUP1)) == 0)
        ;

    adcGetData(adcREG1, adcGROUP1, adcRawDataBuffer);

    // map received values
    portSHORT i = 0;
    for (; i < ADC_FIFO_SIZE; ++i)
    {
        // check pins in mapper array
        portSHORT j = 0;
        for (; j < ADC_FIFO_SIZE; ++j)
        {
            if (adcRawDataBuffer[i].id == mapper[j].adc_pin)
            {
                portSHORT dataIdx = mapper[j].data_idx;
                adcData->values[dataIdx] = adcRawDataBuffer[i].value;
            }
        }
    }
}

AdcValue_t getMaxADCValue()
{
    return (AdcValue_t)0xff;
}

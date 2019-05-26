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

#define ADC_FIFO_SIZE 6

typedef uint16 AdcValue_t;

// uint16 is adcData_t->value type
typedef struct AdcDataValues
{
    AdcValue_t values[ADC_FIFO_SIZE];
} AdcDataValues;

typedef enum
{
    /* First 4 for wheels */
    FL_WHEEL_IDX = 0,  // front left
    FR_WHEEL_IDX = 1,  // front right
    BL_WHEEL_IDX = 2,  // back left
    BR_WHEEL_IDX = 3,   // back right

    /* other values */
    COMPRESSOR_IDX = 4,
    BATTERY_IDX = 5
} ADC_DATA_IDX;

typedef enum ADC_PINS_VALUES
{
    FL_WHEEL_ADC_PIN = 8,
    FR_WHEEL_ADC_PIN = 14,
    BL_WHEEL_ADC_PIN = 13,
    BR_WHEEL_ADC_PIN = 11,

    COMPRESSOR_ADC_PIN = 16,
    BATTERY_ADC_PIN = 23
} ADC_PINS;

/*
 * Mapper
 */
typedef struct idx_value
{
    short data_idx;
    short adc_pin;
} _idx_value;


void initializeADC();
void getADCDataValues(AdcDataValues* adcData);

#endif /* SOURCE_INCLUDES_ADCCONTROLLER_H_ */

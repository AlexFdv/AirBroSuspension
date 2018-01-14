/*
 * ADCController.c
 *
 *  Created on: 7 ñ³÷. 2018 ð.
 *      Author: Alex
 */


#include "ADCController.h"
#include "adc.h"

#include "FreeRTOS.h"
#include "os_semphr.h"

bool doConversion[WHEELS_COUNT];
xSemaphoreHandle semaphore;

void initializeADC()
{
    adcInit();

    semaphore = xSemaphoreCreateCounting(WHEELS_COUNT, 0);

    short i = 0;
    for (; i < WHEELS_COUNT; ++i)
    {
        doConversion[i] = false;
    }
}

void startADCConversion(short wheelNumber)
{
    doConversion[wheelNumber] = true;
    xSemaphoreGive(semaphore);
}

void stopADCConversion(short wheelNumber)
{
    doConversion[wheelNumber] = false;
}

bool checkConversions()
{
    short i = 0;
    for (; i < WHEELS_COUNT; ++i)
    {
        if (doConversion[i])
            return true;
    }
    return false;
}


void getADCValues(adcData_t* adcData)
{
    if (!checkConversions())
    {
        xSemaphoreTake(semaphore, portMAX_DELAY);
    }

    // it is not "continue" conversion
    adcStartConversion(adcREG1, adcGROUP1);
    while ((adcIsConversionComplete(adcREG1, adcGROUP1)) == 0)
                    ;
    adcGetData(adcREG1, adcGROUP1, adcData);
}

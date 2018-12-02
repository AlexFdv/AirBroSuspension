/*
 * Diagnostics.h
 *
 *  Created on: 17 ���. 2018 �.
 *      Author: Alex
 */

#ifndef INCLUDE_APPLICATION_DIAGNOSTIC_H_
#define INCLUDE_APPLICATION_DIAGNOSTIC_H_

#include "ConstantsCommon.h"
#include "ADCController.h"

typedef struct
{
    bool wheels_stats[WHEELS_COUNT * 2];
    AdcDataValues adc_values;

} Diagnostic;

#endif /* INCLUDE_APPLICATION_DIAGNOSTIC_H_ */

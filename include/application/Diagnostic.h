/*
 * Diagnostics.h
 *
 *  Created on: 17 вер. 2018 р.
 *      Author: Alex
 */

#ifndef _APS_DIAGNOSTIC_H_
#define _APS_DIAGNOSTIC_H_

#include "ConstantsCommon.h"
#include "ADCController.h"

typedef struct
{
    bool wheel_pins[WHEELS_COUNT * 2];
    bool wheels_stats[WHEELS_COUNT * 2];
    AdcDataValues adc_values;

} Diagnostic;

#endif /* _APS_DIAGNOSTIC_H_ */

/*
 * Levels.h
 *
 *  Created on: 19 ����. 2017 �.
 *      Author: Alex
 */

#ifndef _APS_LEVELS_H_
#define _APS_LEVELS_H_

#include "ConstantsCommon.h"
#include "ADCController.h"

typedef struct _LevelValues
{
    AdcValue_t wheels[WHEELS_COUNT];
} LevelValues;


#endif /* _APS_LEVELS_H_ */

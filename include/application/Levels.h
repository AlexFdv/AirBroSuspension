/*
 * Levels.h
 *
 *  Created on: 19 лист. 2017 р.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_LEVELS_H_
#define SOURCE_INCLUDES_LEVELS_H_

#include "ConstantsCommon.h"
#include "ADCController.h"

typedef struct _LevelValues
{
    AdcValue_t wheels[WHEELS_COUNT];
} LevelValues;

#endif /* SOURCE_INCLUDES_LEVELS_H_ */

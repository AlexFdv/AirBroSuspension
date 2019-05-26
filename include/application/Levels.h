/*
 * Levels.h
 *
 *  Created on: 19 ����. 2017 �.
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


typedef enum
{
    LevelMin,
    LevelMax
} LEVEL_TYPE;

#endif /* SOURCE_INCLUDES_LEVELS_H_ */

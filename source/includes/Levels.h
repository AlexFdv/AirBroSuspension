/*
 * Levels.h
 *
 *  Created on: 19 лист. 2017 р.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_LEVELS_H_
#define SOURCE_INCLUDES_LEVELS_H_

#include "Constants.h"

typedef struct _LevelValues
{
    portSHORT fl_wheel;
    portSHORT fr_wheel;
    portSHORT bl_wheel;
    portSHORT br_wheel;
} LevelValues;

LevelValues Levels[WHEELS_COUNT];


#endif /* SOURCE_INCLUDES_LEVELS_H_ */

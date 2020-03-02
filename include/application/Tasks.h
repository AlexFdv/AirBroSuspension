/*
 * Tasks.h
 *
 *  Created on: May 26, 2019
 *      Author: oleg
 */

#ifndef _APS_TASKS_H_
#define _APS_TASKS_H_

#include <stdbool.h>
#include <stdint.h>

#include "ADCController.h"
#include "Levels.h"
#include "Settings.h"

typedef void (*ErrorHandler)(void);

bool tasksInit(ErrorHandler errHandler);
void commandReceivedCallback(uint8_t* receivedCommand, short length);

//@todo: separate these functions to another unit
bool getBatteryVoltage(long* const retVoltage);
bool getCompressorPressure(AdcValue_t* const retLevel);
bool getCurrentWheelsLevelsValues(LevelValues* const retLevels);
bool setCachedWheelLevel(uint8_t levelNumber, LevelValues values);
const LevelValues* getCachedWheelLevels(void);
Settings* getSettings(void);

#endif /* _APS_TASKS_H_ */

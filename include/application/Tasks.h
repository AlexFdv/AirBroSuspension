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


typedef enum
{
    SettingMin,
    SettingMax
} SETTING_TYPE;

typedef void (*ErrorHandler)(void);

bool tasksInit(ErrorHandler errHandler);
void commandReceivedCallback(uint8_t* receivedCommand, short length);

#endif /* _APS_TASKS_H_ */

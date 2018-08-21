/*
 * Constants.h
 *
 *  Created on: 14 ����. 2017 �.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_CONSTANTSCOMMON_H_
#define SOURCE_INCLUDES_CONSTANTSCOMMON_H_

#define MAX_COMMAND_LEN 10
#define COMMAND_ARGS_LIMIT 5

#define LEVELS_COUNT 3  // BLOCK_SIZE / sizeof(levels)
#define WHEELS_COUNT 4
#define DEFAULT_PRIORITY 3
#define WHEEL_TIMER_TIMEOUT_SEC 2

typedef enum
{
    FL_WHEEL = 0,  // front left
    FR_WHEEL = 1,  // front right
    BL_WHEEL = 2,  // back left
    BR_WHEEL = 3   // back right
} WHEEL_IDX;

#endif /* SOURCE_INCLUDES_CONSTANTSCOMMON_H_ */

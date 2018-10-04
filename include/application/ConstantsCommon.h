/*
 * Constants.h
 *
 *  Created on: 14 ����. 2017 �.
 *      Author: Alex
 */

#ifndef SOURCE_INCLUDES_CONSTANTSCOMMON_H_
#define SOURCE_INCLUDES_CONSTANTSCOMMON_H_

#define MAX_COMMAND_LEN 10
#define MAX_COMMANDS_QUEUE_LEN 5
#define COMMAND_ARGS_LIMIT 5
#define WHEELS_LEVELS_DEVIATION 4
#define WHEELS_LEVELS_THRESHOLD WHEELS_LEVELS_DEVIATION/4

#define LEVELS_COUNT 3  // BLOCK_SIZE / sizeof(levels)
#define WHEELS_COUNT 4

#define WHEEL_TIMER_TIMEOUT_SEC 4

#define TASK_LOW_PRIORITY 2
#define TASK_DEFAULT_PRIORITY 3
#define TASK_HIGH_PRIORITY 3

// it is better don't change the values, or find usages of it.
typedef enum
{
    FL_WHEEL = 0,  // front left
    FR_WHEEL = 1,  // front right
    BL_WHEEL = 2,  // back left
    BR_WHEEL = 3   // back right
} WHEEL_IDX;

#endif /* SOURCE_INCLUDES_CONSTANTSCOMMON_H_ */

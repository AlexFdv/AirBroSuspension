#ifndef _APS_SETTINGS_H_
#define _APS_SETTINGS_H_

#include <stdint.h>
#include "Levels.h"

// total size is 24 bytes. If change - then change in halcogen as well
typedef struct _Settings
{
    uint16_t compressor_preasure_max;
    uint16_t compressor_preasure_min;
    LevelValues levels_values_max;  // 4*uint16
    LevelValues levels_values_min;  // 4*uint16
    uint16_t reserved_for_future[2];
} Settings;

#endif /* _APS_SETTINGS_H_ */

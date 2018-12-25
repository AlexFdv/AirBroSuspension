#ifndef SOURCE_INCLUDES_SETTINGS_H_
#define SOURCE_INCLUDES_SETTINGS_H_

#include "Levels.h"

// total size is 24 bytes. If change - then change in halcogen as well
typedef struct _Settings
{
    uint16 compressor_preasure_max;
    uint16 compressor_preasure_min;
    LevelValues levels_values_max;  // 4*uint16
    LevelValues levels_values_min;  // 4*uint16
    uint16 reserved_for_future[2];
} Settings;

#endif /* SOURCE_INCLUDES_SETTINGS_H_ */

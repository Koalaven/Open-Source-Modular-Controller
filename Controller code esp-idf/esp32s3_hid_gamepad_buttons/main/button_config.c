#include "button_config.h"

button_config_t button_map[NUM_BUTTONS] = {
    { "A",  GPIO_NUM_1,  (1 << 0) },
    { "B",  GPIO_NUM_2,  (1 << 1) },
    { "X",  GPIO_NUM_3,  (1 << 2) },
    { "Y",  GPIO_NUM_4,  (1 << 3) },
};

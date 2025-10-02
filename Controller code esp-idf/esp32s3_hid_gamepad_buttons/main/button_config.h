#pragma once
#include "driver/gpio.h"

typedef struct {
    const char *name;
    gpio_num_t pin;
    uint16_t hid_mask;
} button_config_t;

#define NUM_BUTTONS 4

extern button_config_t button_map[NUM_BUTTONS];

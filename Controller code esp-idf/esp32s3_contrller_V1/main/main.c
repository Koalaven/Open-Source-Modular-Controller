#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_adc/adc_continuous.h"
#include "esp_system.h"
#include "tusb.h"
#include "class/hid/hid_device.h"

// Tag for ESP logging
static const char *TAG = "controller_v1";

// GPIO pin definitions for controller buttons
#define START_PIN 35
#define SELECT_PIN 36
#define HOME_PIN 21
#define R1_PIN 17
#define L1_PIN 16
#define R2_PIN 27
#define L2_PIN 26
#define R3_PIN 25
#define L3_PIN 33
#define A_PIN 32
#define B_PIN 34
#define X_PIN 39
#define Y_PIN 14
#define UP_PIN 13
#define DOWN_PIN 12
#define LEFT_PIN 15
#define RIGHT_PIN 2

// Analog stick deadzone and polling rate
#define DEADZONE 8
#define POLL_DELAY_MS 10

#define ADC_UNIT ADC_UNIT_1
static adc_continuous_handle_t adc_handle = NULL;

// Define ADC channels for analog inputs
static const adc_channel_t adc_channels[] = {
    ADC_CHANNEL_0,
    ADC_CHANNEL_3,
    ADC_CHANNEL_6,
    ADC_CHANNEL_7
};

// Gamepad HID report structure
typedef struct {
    int8_t x;   // Left stick X-axis
    int8_t y;   // Left stick Y-axis
    int8_t z;   // Right stick X-axis
    int8_t rz;  // Right stick Y-axis
    int8_t rx;  // Left trigger
    int8_t ry;  // Right trigger
    uint8_t hat; // D-pad hat value
    uint32_t buttons; // Button bitmask
} hid_gamepad_report_t;

// Map integer from one range to another (used for joystick normalization)
static int32_t map_int32(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    if (in_max == in_min) return out_min; // Avoid divide-by-zero
    int64_t res = (int64_t)(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    return (int32_t)res;
}

// Initialize GPIO pins for buttons
static void gpio_inputs_init(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, // Enable pull-ups for button inputs
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pin_bit_mask = ((1ULL<<R1_PIN)|(1ULL<<L1_PIN)|(1ULL<<R2_PIN)|(1ULL<<L2_PIN)|
                         (1ULL<<R3_PIN)|(1ULL<<L3_PIN)|(1ULL<<A_PIN)|(1ULL<<B_PIN)|
                         (1ULL<<X_PIN)|(1ULL<<Y_PIN)|(1ULL<<UP_PIN)|(1ULL<<DOWN_PIN)|
                         (1ULL<<LEFT_PIN)|(1ULL<<RIGHT_PIN)|(1ULL<<START_PIN)|
                         (1ULL<<SELECT_PIN)|(1ULL<<HOME_PIN))
    };
    gpio_config(&io_conf);
}

// Initialize ADC continuous mode with four channels
static void adc_init_continuous(void) {
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024, // Buffer for ADC samples
        .conv_frame_size = 256,     // Frame size for reading samples
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle));

    // Configure ADC channel patterns
    adc_digi_pattern_config_t adc_patterns[4] = {0};
    for (int i = 0; i < 4; i++) {
        adc_patterns[i].atten = ADC_ATTEN_DB_12; // 12 dB attenuation for full range
        adc_patterns[i].channel = adc_channels[i];
        adc_patterns[i].unit = ADC_UNIT_1;
        adc_patterns[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
    }

    // Set up continuous ADC configuration
    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000, // 20 kHz sampling frequency
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2, // TYPE2 output format for ESP32-S3
        .pattern_num = 4,
        .adc_pattern = adc_patterns,
    };

    ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &dig_cfg));
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));
}

// Read ADC values continuously into buffer
static void read_adc_values(int *values) {
    uint8_t result[256];
    uint32_t bytes_read = 0;

    // Attempt to read ADC data
    if (adc_continuous_read(adc_handle, result, sizeof(result), &bytes_read, 0) == ESP_OK) {
        for (uint32_t i = 0; i + sizeof(adc_digi_output_data_t) <= bytes_read; i += sizeof(adc_digi_output_data_t)) {
            adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
            int channel = p->type2.channel;
            int value = p->type2.data;
            // Match the channel and store corresponding value
            for (int j = 0; j < 4; j++) {
                if (channel == adc_channels[j]) values[j] = value;
            }
        }
    }
}

// Read button state (returns true when pressed)
static bool read_button(uint32_t pin) { return gpio_get_level(pin) == 0; }

// Send HID gamepad report via TinyUSB
static void send_gamepad_report(uint8_t instance, uint8_t report_id, int8_t lx, int8_t ly, int8_t rx, int8_t ry, int8_t l2, int8_t r2, uint8_t hat, uint32_t buttons) {
    if (tud_ready()) {
        // Send gamepad report using TinyUSB high-level HID API
        tud_hid_n_gamepad_report(instance, report_id, lx, ly, rx, ry, l2, r2, hat, buttons);
    }
}

// Main controller task
static void app_task(void *arg) {
    int adc_values[4] = {2048, 2048, 2048, 2048}; // Initialize with mid-range values

    while (1) {
        // Read all ADC channels
        read_adc_values(adc_values);

        // Convert ADC readings (0–4095) to joystick range (-127–127)
        int8_t lx = map_int32(adc_values[2], 0, 4095, -127, 127);
        int8_t ly = map_int32(adc_values[3], 0, 4095, -127, 127);
        int8_t rx = map_int32(adc_values[0], 0, 4095, -127, 127);
        int8_t ry = map_int32(adc_values[1], 0, 4095, -127, 127);

        // Apply deadzone filtering to avoid drift
        if (abs(lx) < DEADZONE) lx = 0;
        if (abs(ly) < DEADZONE) ly = 0;
        if (abs(rx) < DEADZONE) rx = 0;
        if (abs(ry) < DEADZONE) ry = 0;

        // Collect button presses into a single bitfield
        uint32_t buttons = 0;
        if (read_button(A_PIN)) buttons |= (1U<<0);
        if (read_button(B_PIN)) buttons |= (1U<<1);
        if (read_button(X_PIN)) buttons |= (1U<<2);
        if (read_button(Y_PIN)) buttons |= (1U<<3);
        if (read_button(R1_PIN)) buttons |= (1U<<4);
        if (read_button(L1_PIN)) buttons |= (1U<<5);
        if (read_button(R3_PIN)) buttons |= (1U<<6);
        if (read_button(L3_PIN)) buttons |= (1U<<7);
        if (read_button(START_PIN)) buttons |= (1U<<8);
        if (read_button(SELECT_PIN)) buttons |= (1U<<9);
        if (read_button(HOME_PIN)) buttons |= (1U<<10);

        // Process D-pad directions into HAT switch value
        bool up = read_button(UP_PIN);
        bool down = read_button(DOWN_PIN);
        bool left = read_button(LEFT_PIN);
        bool right = read_button(RIGHT_PIN);

        uint8_t hat = 8; // 8 = neutral
        if (up && left) hat = 7; else if (up && right) hat = 1;
        else if (down && left) hat = 5; else if (down && right) hat = 3;
        else if (up) hat = 0; else if (right) hat = 2;
        else if (down) hat = 4; else if (left) hat = 6;

        // Analog trigger values (binary press simulated as 0 or 127)
        int8_t l2 = read_button(L2_PIN) ? 127 : 0;
        int8_t r2 = read_button(R2_PIN) ? 127 : 0;

        // Send HID report with all gamepad inputs
        send_gamepad_report(0, 0, lx, ly, rx, ry, l2, r2, hat, buttons);

        // Log inputs for debugging
        ESP_LOGI(TAG, "BTN=0x%08lx LX=%d LY=%d RX=%d RY=%d HAT=%d", (unsigned long)buttons, lx, ly, rx, ry, hat);

        // Delay before next poll
        vTaskDelay(pdMS_TO_TICKS(POLL_DELAY_MS));
    }
}

// Application entry point
void app_main(void) {
    ESP_LOGI(TAG, "Controller-V1 using adc_continuous TYPE2 and tud_hid_n_gamepad_report()");
    gpio_inputs_init();     // Initialize button inputs
    adc_init_continuous();  // Start continuous ADC sampling
    tusb_init();            // Initialize TinyUSB stack
    xTaskCreate(app_task, "app_task", 4096, NULL, 5, NULL); // Start main task
}
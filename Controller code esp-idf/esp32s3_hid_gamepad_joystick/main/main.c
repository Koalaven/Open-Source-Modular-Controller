#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "esp_adc/adc_continuous.h"

#define TAG "HID_GAMEPAD"

// HID report descriptor (gamepad with 2 axes + 2 buttons)
static const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(1))
};

// Gamepad state
typedef struct {
    int8_t x;   // X axis
    int8_t y;   // Y axis
    uint8_t buttons;
} __attribute__((packed)) gamepad_report_t;

static adc_continuous_handle_t adc_handle;

// Convert raw ADC to joystick axis
static int8_t map_adc_to_axis(uint32_t adc_raw) {
    int32_t axis = ((int32_t)adc_raw * 255 / 4095) - 127;
    if (axis > 127) axis = 127;
    if (axis < -127) axis = -127;
    return (int8_t)axis;
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting USB HID Gamepad");

    // TinyUSB init
    tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .string_descriptor_count = 0,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_hid_config_t hid_cfg = {
        .iface = TINYUSB_HID_ITF_0,
        .desc = hid_report_descriptor,
        .callback = NULL,
    };
    ESP_ERROR_CHECK(tinyusb_hid_init(&hid_cfg));

    // --- ADC config ---
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = 256,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle));

    adc_digi_pattern_config_t patterns[2] = {0};
    patterns[0].atten = ADC_ATTEN_DB_11;
    patterns[0].channel = ADC_CHANNEL_0;  // GPIO1
    patterns[0].unit = ADC_UNIT_1;
    patterns[0].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

    patterns[1].atten = ADC_ATTEN_DB_11;
    patterns[1].channel = ADC_CHANNEL_1;  // GPIO2
    patterns[1].unit = ADC_UNIT_1;
    patterns[1].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 1000,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
        .pattern_num = 2,
        .adc_pattern = patterns,
    };

    ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &dig_cfg));
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));

    // --- HID loop ---
    uint8_t result[256];
    while (1) {
        uint32_t ret_num = 0;
        esp_err_t ret = adc_continuous_read(adc_handle, result, sizeof(result), &ret_num, 1000);
        if (ret == ESP_OK && ret_num >= sizeof(adc_digi_output_data_t) * 2) {
            adc_digi_output_data_t *p = (adc_digi_output_data_t *)result;
            uint32_t raw_x = p[0].type2.data;
            uint32_t raw_y = p[1].type2.data;

            gamepad_report_t rpt = {
                .x = map_adc_to_axis(raw_x),
                .y = map_adc_to_axis(raw_y),
                .buttons = 0x00
            };

            tud_hid_report(1, &rpt, sizeof(rpt));
            ESP_LOGI(TAG, "Report sent: X=%d Y=%d", rpt.x, rpt.y);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

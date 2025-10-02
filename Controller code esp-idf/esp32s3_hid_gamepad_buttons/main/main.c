#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "driver/gpio.h"
#include "button_config.h"

#define TAG "HID_GAMEPAD"

static const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(1))
};

typedef struct {
    int8_t x;
    int8_t y;
    uint16_t buttons;
} __attribute__((packed)) gamepad_report_t;

void app_main(void) {
    ESP_LOGI(TAG, "Starting USB HID Gamepad");

    tinyusb_config_t tusb_cfg = {0};
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_hid_config_t hid_cfg = {
        .iface = TINYUSB_HID_ITF_0,
        .desc = hid_report_descriptor,
        .callback = NULL,
    };
    ESP_ERROR_CHECK(tinyusb_hid_init(&hid_cfg));

    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << button_map[i].pin),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        gpio_config(&io_conf);
    }

    while (1) {
        uint16_t buttons_state = 0;
        for (int i = 0; i < NUM_BUTTONS; i++) {
            int level = gpio_get_level(button_map[i].pin);
            if (level == 0) {
                buttons_state |= button_map[i].hid_mask;
            }
        }

        gamepad_report_t rpt = {
            .x = 0,
            .y = 0,
            .buttons = buttons_state
        };

        tud_hid_report(1, &rpt, sizeof(rpt));
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "esp_adc/adc_continuous.h"

// Simple USB HID Gamepad (stub axes + buttons)
void app_main(void) {
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
        .desc = NULL,
        .callback = NULL,
    };
    ESP_ERROR_CHECK(tinyusb_hid_init(&hid_cfg));

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf("USB HID Gamepad running...\\n");
    }
}

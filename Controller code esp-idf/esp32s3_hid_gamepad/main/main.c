#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "tusb.h"
#include "sdkconfig.h"

static const char *TAG = "hid_gamepad";

#define PIN_LX_ADC_CH    ADC_CHANNEL_0
#define PIN_LY_ADC_CH    ADC_CHANNEL_1
#define PIN_RX_ADC_CH    ADC_CHANNEL_2
#define PIN_RY_ADC_CH    ADC_CHANNEL_3
#define PIN_R2_ADC_CH    ADC_CHANNEL_4

static const int btn_pins[] = { 17, 18, 3, 6, 15, 16 };

#define NUM_PHYSICAL_BUTTONS (sizeof(btn_pins) / sizeof(btn_pins[0]))
#define BUTTON_COUNT 32
#define HID_REPORT_LEN 10
#define HID_REPORT_ID 0x01

static uint8_t adc_to_u8(int raw) {
    if (raw < 0) raw = 0;
    if (raw > 4095) raw = 4095;
    return (uint8_t)((raw * 255) / 4095);
}

void tud_mount_cb(void) { ESP_LOGI(TAG, "USB mounted"); }
void tud_umount_cb(void) { ESP_LOGI(TAG, "USB unmounted"); }
void tud_suspend_cb(bool remote_wakeup_en) { ESP_LOGI(TAG, "USB suspended"); }
void tud_resume_cb(void) { ESP_LOGI(TAG, "USB resumed"); }

uint8_t const hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_BUTTONS(32),
    TUD_HID_REPORT_DESC_AXIS(6),
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance, uint16_t *length) {
    *length = sizeof(hid_report_descriptor);
    return hid_report_descriptor;
}

static void gpio_init(void) {
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    for (size_t i = 0; i < NUM_PHYSICAL_BUTTONS; ++i) {
        io_conf.pin_bit_mask = (1ULL << btn_pins[i]);
        gpio_config(&io_conf);
    }
}

static void adc_init_all(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(PIN_LX_ADC_CH, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(PIN_LY_ADC_CH, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(PIN_RX_ADC_CH, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(PIN_RY_ADC_CH, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(PIN_R2_ADC_CH, ADC_ATTEN_DB_11);
}

static void send_hid_report_task(void *arg) {
    uint8_t report[HID_REPORT_LEN];
    while (1) {
        if (!tud_ready()) { vTaskDelay(pdMS_TO_TICKS(50)); continue; }
        uint32_t buttons = 0;
        for (size_t i = 0; i < NUM_PHYSICAL_BUTTONS && i < 32; ++i) {
            if (gpio_get_level(btn_pins[i]) == 0) buttons |= (1UL << i);
        }
        report[0] = buttons & 0xFF;
        report[1] = (buttons >> 8) & 0xFF;
        report[2] = (buttons >> 16) & 0xFF;
        report[3] = (buttons >> 24) & 0xFF;
        report[4] = adc_to_u8(adc1_get_raw(PIN_LX_ADC_CH));
        report[5] = adc_to_u8(adc1_get_raw(PIN_LY_ADC_CH));
        report[6] = adc_to_u8(adc1_get_raw(PIN_RX_ADC_CH));
        report[7] = adc_to_u8(adc1_get_raw(PIN_RY_ADC_CH));
        report[8] = adc_to_u8(adc1_get_raw(PIN_R2_ADC_CH));
        report[9] = 8; // hat neutral
        if (tud_hid_ready()) tud_hid_report(HID_REPORT_ID, report, HID_REPORT_LEN);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting HID gamepad (ESP-IDF + TinyUSB)");
    gpio_init();
    adc_init_all();
    tusb_init();
    xTaskCreate(send_hid_report_task, "hid_report_task", 4096, NULL, 5, NULL);
}

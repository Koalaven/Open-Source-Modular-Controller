#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#define I2C_SLAVE_PORT     I2C_NUM_0
#define I2C_SLAVE_RX_BUF   128
#define I2C_SLAVE_TX_BUF   128

#define I2C_SLAVE_SDA      CONFIG_PCAL6416A_I2C_SDA
#define I2C_SLAVE_SCL      CONFIG_PCAL6416A_I2C_SCL
#define I2C_SLAVE_ADDR     CONFIG_PCAL6416A_I2C_ADDR
#define INT_GPIO           CONFIG_PCAL6416A_INT_PIN

static uint8_t regs[0x50];
static uint8_t pointer_reg = 0x00;

static uint8_t prev_input0 = 0, prev_input1 = 0;
static uint8_t latched_input0 = 0, latched_input1 = 0;

static uint8_t next_reg(uint8_t reg) {
    switch (reg) {
        case 0x00: return 0x01; case 0x01: return 0x00;
        case 0x02: return 0x03; case 0x03: return 0x02;
        case 0x04: return 0x05; case 0x05: return 0x04;
        case 0x06: return 0x07; case 0x07: return 0x06;
        case 0x40: return 0x41; case 0x41: return 0x40;
        case 0x42: return 0x43; case 0x43: return 0x42;
        case 0x44: return 0x45; case 0x45: return 0x44;
        case 0x46: return 0x47; case 0x47: return 0x46;
        case 0x48: return 0x49; case 0x49: return 0x48;
        case 0x4A: return 0x4B; case 0x4B: return 0x4A;
        case 0x4C: return 0x4D; case 0x4D: return 0x4C;
        default:   return reg + 1;
    }
}

static void init_registers() {
    memset(regs, 0, sizeof(regs));
    regs[0x02] = 0xFF; regs[0x03] = 0xFF;
    regs[0x06] = 0xFF; regs[0x07] = 0xFF;
    regs[0x40] = regs[0x41] = regs[0x42] = regs[0x43] = 0xFF;
    regs[0x48] = regs[0x49] = 0xFF;
    regs[0x4A] = regs[0x4B] = 0xFF;
    regs[0x4F] = 0x00;
    prev_input0 = regs[0x00];
    prev_input1 = regs[0x01];
    latched_input0 = 0;
    latched_input1 = 0;
}

static void update_interrupt() {
    uint8_t input0 = regs[0x00], input1 = regs[0x01];
    uint8_t mask0  = regs[0x4A], mask1  = regs[0x4B];
    uint8_t latch0 = regs[0x44], latch1 = regs[0x45];
    uint8_t changed0 = (input0 ^ prev_input0) & ~mask0;
    uint8_t changed1 = (input1 ^ prev_input1) & ~mask1;
    if (latch0) latched_input0 |= changed0; else latched_input0 = changed0;
    if (latch1) latched_input1 |= changed1; else latched_input1 = changed1;
    regs[0x4C] = latched_input0;
    regs[0x4D] = latched_input1;
    gpio_set_level(INT_GPIO, (latched_input0 || latched_input1) ? 0 : 1);
}

static void clear_interrupt_on_read(uint8_t reg) {
    if (reg == 0x00 || reg == 0x01) {
        prev_input0 = regs[0x00];
        prev_input1 = regs[0x01];
        latched_input0 = latched_input1 = 0;
        regs[0x4C] = regs[0x4D] = 0;
        gpio_set_level(INT_GPIO, 1);
    }
}

static void emulator_task(void *arg) {
    uint8_t buf[32];
    while (1) {
        int len = i2c_slave_read_buffer(I2C_SLAVE_PORT, buf, sizeof(buf), 10 / portTICK_PERIOD_MS);
        if (len > 0) {
            if (len == 1) {
                pointer_reg = buf[0];
            } else {
                pointer_reg = buf[0];
                for (int i = 1; i < len; i++) {
                    if (pointer_reg < sizeof(regs)) {
                        regs[pointer_reg] = buf[i];
                        pointer_reg = next_reg(pointer_reg);
                    }
                }
            }
            update_interrupt();
        }
        if (pointer_reg < sizeof(regs)) {
            uint8_t val = regs[pointer_reg];
            clear_interrupt_on_read(pointer_reg);
            pointer_reg = next_reg(pointer_reg);
            i2c_reset_tx_fifo(I2C_SLAVE_PORT);
            i2c_slave_write_buffer(I2C_SLAVE_PORT, &val, 1, 10 / portTICK_PERIOD_MS);
        }
        vTaskDelay(1);
    }
}

void app_main(void) {
    printf("PCAL6416A Pure Emulation Starting...\n");
    init_registers();
    gpio_reset_pin(INT_GPIO);
    gpio_set_direction(INT_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(INT_GPIO, 1);
    i2c_config_t conf = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = I2C_SLAVE_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SLAVE_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave = {
            .addr_10bit_en = 0,
            .slave_addr = I2C_SLAVE_ADDR
        }
    };
    i2c_param_config(I2C_SLAVE_PORT, &conf);
    i2c_driver_install(I2C_SLAVE_PORT, I2C_MODE_SLAVE, I2C_SLAVE_RX_BUF, I2C_SLAVE_TX_BUF, 0);
    xTaskCreate(emulator_task, "emulator_task", 4096, NULL, 10, NULL);
}

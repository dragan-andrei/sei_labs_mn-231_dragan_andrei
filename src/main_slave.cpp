#include <Arduino.h>

#include "app_i2c_slave.h"
#include "configs.h"
#include "ctrl_stdio.h"

namespace {

void print_boot_banner() {
    ctrl_stdio_print_text("\n================================================\n");
    ctrl_stdio_print_text("   LAB 7.1 — I2C SLAVE (ESP32)\n");
    ctrl_stdio_print_text("   Achiziție HC-SR04 + răspuns I2C cu CHECKSUM\n");
    ctrl_stdio_print_text("================================================\n");
}

}  // namespace

void setup() {
    ctrl_stdio_init(SERIAL_BAUDRATE);
    print_boot_banner();
    app_i2c_slave_init();
}

void loop() {
    vTaskDelete(nullptr);
}

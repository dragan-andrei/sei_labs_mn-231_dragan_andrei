#include <Arduino.h>

#include "app_i2c_master.h"
#include "configs.h"
#include "ctrl_stdio.h"

namespace {

void print_boot_banner() {
    ctrl_stdio_print_text("\n================================================\n");
    ctrl_stdio_print_text("   LAB 7.1 — I2C MASTER (ESP32)\n");
    ctrl_stdio_print_text("   Interoghează slave-ul HC-SR04 prin magistrala I2C\n");
    ctrl_stdio_print_text("================================================\n");
}

}  // namespace

void setup() {
    ctrl_stdio_init(SERIAL_BAUDRATE);
    print_boot_banner();
    app_i2c_master_init();
}

void loop() {
    // În FreeRTOS pe ESP32 logica rulează în task-uri; loop-ul nu este folosit.
    vTaskDelete(nullptr);
}

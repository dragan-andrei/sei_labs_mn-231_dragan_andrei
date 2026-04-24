#include <Arduino.h>

#include "app_mqtt_node.h"
#include "configs.h"
#include "ctrl_stdio.h"

namespace {
void print_boot_banner() {
    ctrl_stdio_print_text("\n================================================\n");
    ctrl_stdio_print_text("   LAB 7.3 — MQTT IoT NODE (ESP32)\n");
    ctrl_stdio_print_text("   DHT22 (telemetry) + LED (actuator) via HiveMQ\n");
    ctrl_stdio_print_text("================================================\n");
}
}  // namespace

void setup() {
    ctrl_stdio_init(SERIAL_BAUDRATE);
    print_boot_banner();
    app_mqtt_node_start();
}

void loop() {
    vTaskDelete(nullptr);
}

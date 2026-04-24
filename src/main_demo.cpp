// Build alternativ: rulează MASTER-ul (pe I²C-ul 0 / `Wire`) ȘI SLAVE-ul
// (pe I²C-ul 1 / `Wire1`) pe același ESP32. În schema Wokwi cele două magistrale
// sunt unite extern prin jumperi (SDA master <-> SDA slave, SCL master <-> SCL
// slave), astfel încât protocolul complet se poate simula în mediul Wokwi — cu
// un singur firmware încărcat — fără a avea nevoie de două plăci fizice.
//
// Pentru laborator (fizic) se folosesc build-urile separate `esp32_master` și
// `esp32_slave`, flash-uite pe două plăci distincte.

#include <Arduino.h>
#include <Wire.h>

#include "app_i2c_master.h"
#include "app_i2c_slave.h"
#include "configs.h"
#include "ctrl_stdio.h"

namespace {

void print_boot_banner() {
    ctrl_stdio_print_text("\n================================================\n");
    ctrl_stdio_print_text("   LAB 7.1 — I2C DEMO LOOPBACK (ESP32, un chip)\n");
    ctrl_stdio_print_text("   Master pe Wire (GPIO 21/22) <-> Slave pe Wire1 (GPIO 16/17)\n");
    ctrl_stdio_print_text("================================================\n");
}

}  // namespace

void setup() {
    ctrl_stdio_init(SERIAL_BAUDRATE);
    print_boot_banner();

    // Slave-ul pornește primul, ca să fie pregătit când master-ul începe să
    // interogheze. `Wire1` este legat extern (în diagrama Wokwi) de `Wire`.
    app_i2c_slave_init(&Wire1,
                       I2C_DEMO_SLAVE_SDA_PIN,
                       I2C_DEMO_SLAVE_SCL_PIN,
                       I2C_SLAVE_ADDRESS);

    app_i2c_master_init(&Wire,
                        I2C_DEMO_MASTER_SDA_PIN,
                        I2C_DEMO_MASTER_SCL_PIN,
                        I2C_SLAVE_ADDRESS);
}

void loop() {
    vTaskDelete(nullptr);
}

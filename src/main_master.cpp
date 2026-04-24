#include <Arduino.h>
#include <HardwareSerial.h>

#include "app_uart_master.h"
#include "configs.h"
#include "ctrl_stdio.h"

namespace {
void print_boot_banner() {
    ctrl_stdio_print_text("\n================================================\n");
    ctrl_stdio_print_text("   LAB 7.2 — UART MASTER (ESP32)\n");
    ctrl_stdio_print_text("   Protocol binar [STX][ID][SEQ][CMD][LEN][PAYLOAD][CS][ETX]\n");
    ctrl_stdio_print_text("================================================\n");
}
}  // namespace

void setup() {
    ctrl_stdio_init(SERIAL_BAUDRATE);
    print_boot_banner();

    app_uart_master_init(&Serial2,
                         BIN_UART_BAUDRATE,
                         BIN_UART_RX_PIN,
                         BIN_UART_TX_PIN,
                         DEVICE_ID_MASTER,
                         DEVICE_ID_SLAVE);
}

void loop() {
    vTaskDelete(nullptr);
}

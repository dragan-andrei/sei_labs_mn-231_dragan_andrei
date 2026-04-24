// Build alternativ pentru simulatorul Wokwi (single-chip).
//
// Wokwi rulează DOAR o imagine firmware per simulare, așa că nu putem avea
// două ESP32-uri cu două main-uri diferite rulând în același timp. Ca să
// putem totuși vedea **tot** protocolul (TX master, parser pe state machine,
// FIFO, task-uri FreeRTOS, ACK/NACK, retransmitere) într-o singură fereastră
// serial, legăm master-ul și slave-ul pe un **UART virtual in-process**:
//
//   master_uart  ─TX→  [cross-FIFO A]  ─RX→  slave_uart
//                      [cross-FIFO B]  ←RX─  master_uart ←TX─ slave_uart
//
// Fiecare parte citește/scrie ca și cum ar fi un HardwareSerial real, dar
// transportul este un pair de cozi FreeRTOS. Toată logica protocolului
// (encode/checksum/decode/dedup/ACK/NACK/retry) este exercitată identic.
//
// Pentru demonstrația fizică se folosesc env-urile `esp32_master` /
// `esp32_slave` pe două plăci distincte, cu Serial2 cablat cross-over.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "app_uart_master.h"
#include "app_uart_slave.h"
#include "configs.h"
#include "ctrl_stdio.h"
#include "dd_uart.h"

namespace {

QueueHandle_t q_master_to_slave = nullptr;
QueueHandle_t q_slave_to_master = nullptr;

struct endpoint_ctx_t {
    QueueHandle_t rx_from;  // coadă DIN care endpoint-ul ăsta citește
    QueueHandle_t tx_to;    // coadă ÎN care endpoint-ul ăsta scrie
};

endpoint_ctx_t g_master_ep;
endpoint_ctx_t g_slave_ep;

int v_read_ep(void* ctx) {
    endpoint_ctx_t* e = static_cast<endpoint_ctx_t*>(ctx);
    uint8_t byte = 0;
    if (xQueueReceive(e->rx_from, &byte, 0) == pdTRUE) {
        return static_cast<int>(byte);
    }
    return -1;
}

size_t v_write_ep(void* ctx, const uint8_t* data, size_t length) {
    endpoint_ctx_t* e = static_cast<endpoint_ctx_t*>(ctx);
    size_t w = 0;
    for (size_t i = 0; i < length; ++i) {
        if (xQueueSend(e->tx_to, &data[i], 0) != pdTRUE) {
            break;
        }
        ++w;
    }
    return w;
}

void print_boot_banner() {
    ctrl_stdio_print_text("\n================================================\n");
    ctrl_stdio_print_text("   LAB 7.2 — DEMO (Wokwi, UART virtual in-process)\n");
    ctrl_stdio_print_text("   Master + Slave pe același chip,\n");
    ctrl_stdio_print_text("   transport prin FIFO (două cozi FreeRTOS)\n");
    ctrl_stdio_print_text("================================================\n");
}

}  // namespace

void setup() {
    ctrl_stdio_init(SERIAL_BAUDRATE);
    print_boot_banner();

    q_master_to_slave = xQueueCreate(BIN_UART_RX_BUFFER_SIZE, sizeof(uint8_t));
    q_slave_to_master = xQueueCreate(BIN_UART_RX_BUFFER_SIZE, sizeof(uint8_t));
    if (q_master_to_slave == nullptr || q_slave_to_master == nullptr) {
        ctrl_stdio_print_text("[DEMO][EROARE] Nu pot aloca cozi\n");
        return;
    }

    g_slave_ep.rx_from  = q_master_to_slave;
    g_slave_ep.tx_to    = q_slave_to_master;
    g_master_ep.rx_from = q_slave_to_master;
    g_master_ep.tx_to   = q_master_to_slave;

    // Init slave ÎNTÂI (fără hardware), apoi legăm transportul virtual.
    app_uart_slave_init(nullptr,
                        BIN_UART_BAUDRATE,
                        BIN_UART_DEMO_SLAVE_RX_PIN,
                        BIN_UART_DEMO_SLAVE_TX_PIN,
                        DEVICE_ID_SLAVE);
    dd_uart_bind_virtual_transport(app_uart_slave_get_uart(),
                                   v_read_ep,
                                   v_write_ep,
                                   &g_slave_ep);

    app_uart_master_init(nullptr,
                         BIN_UART_BAUDRATE,
                         BIN_UART_DEMO_MASTER_RX_PIN,
                         BIN_UART_DEMO_MASTER_TX_PIN,
                         DEVICE_ID_MASTER,
                         DEVICE_ID_SLAVE);
    dd_uart_bind_virtual_transport(app_uart_master_get_uart(),
                                   v_read_ep,
                                   v_write_ep,
                                   &g_master_ep);

    ctrl_stdio_print_text(
        "[DEMO] Legătura virtuală master<->slave e activă.\n");
}

void loop() {
    vTaskDelete(nullptr);
}

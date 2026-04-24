// Build alternativ pentru simulatorul Wokwi (single-chip).
//
// Wokwi rulează DOAR o imagine firmware per simulare, iar suportul pentru
// modul I²C slave pe ESP32 este parțial — două magistrale hardware legate prin
// jumperi externi nu se sincronizează corect în simulator. Pentru a putea totuși
// observa protocolul complet (encode / checksum / decode / task-uri FreeRTOS /
// mutex / alerta de proximitate) într-o singură fereastră serial, folosim o
// "magistrală virtuală" în-process:
//
//   - slave-ul rulează normal (task-uri FreeRTOS, mutex, buffer encodat), dar
//     fără a se atașa la hardware I²C (`bus = nullptr`);
//   - master-ul de demo citește direct buffer-ul cel mai recent prin
//     `app_i2c_slave_peek_response()` (sub mutex), îl decodează, validează
//     checksum-ul și îl afișează exact ca master-ul fizic.
//
// Astfel toată logica protocolului este exercitată; singurul element "fake" este
// transportul pe fir. Pentru demonstrația fizică a lucrării se folosesc
// env-urile `esp32_master` / `esp32_slave` flash-uite pe două plăci distincte —
// acolo comunicația trece prin Wire-ul real.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "app_i2c_slave.h"
#include "configs.h"
#include "ctrl_i2c_packet.h"
#include "ctrl_stdio.h"
#include "dd_led.h"

namespace {

void print_boot_banner() {
    ctrl_stdio_print_text("\n================================================\n");
    ctrl_stdio_print_text("   LAB 7.1 — I2C DEMO (Wokwi, magistrală virtuală)\n");
    ctrl_stdio_print_text("   Master + Slave pe același chip, transport in-process\n");
    ctrl_stdio_print_text("================================================\n");
}

void demo_print_distance(uint8_t sensor_index, uint16_t cm) {
    if (cm == HCSR04_DISTANCE_ERROR) {
        ctrl_stdio_printf("  Senzor #%u: ---- (timeout / out of range)\n",
                          static_cast<unsigned>(sensor_index + 1));
    } else {
        ctrl_stdio_printf("  Senzor #%u: %u cm\n",
                          static_cast<unsigned>(sensor_index + 1),
                          static_cast<unsigned>(cm));
    }
}

void demo_update_alert(const i2c_packet_t& packet) {
    bool any_close = false;
    for (uint8_t i = 0; i < HCSR04_SENSOR_COUNT; ++i) {
        const uint16_t cm = ctrl_i2c_packet_get_value(&packet, i);
        if (cm != HCSR04_DISTANCE_ERROR && cm < ALERT_PROXIMITY_CM) {
            any_close = true;
            break;
        }
    }
    dd_led_set(ALERT_LED_PIN, any_close ? HIGH : LOW);
    if (any_close) {
        ctrl_stdio_print_text("  >>> ALERTA PROXIMITATE: obiect sub prag! <<<\n");
    }
}

void task_demo_master_poller(void* parameters) {
    (void)parameters;
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(MASTER_POLL_PERIOD_MS);

    uint8_t rx_buffer[I2C_PACKET_MAX_SIZE];

    for (;;) {
        ctrl_stdio_printf(
            "\n[%6lu][MASTER] --- interogare slave 0x%02X (virtual) ---\n",
            millis(),
            static_cast<unsigned>(I2C_SLAVE_ADDRESS));

        const size_t n = app_i2c_slave_peek_response(rx_buffer, sizeof(rx_buffer));
        if (n == 0) {
            ctrl_stdio_print_text("[MASTER][INFO] Buffer gol, reîncerc la următorul tick.\n");
            dd_led_set(ALERT_LED_PIN, LOW);
            vTaskDelayUntil(&last_wake, period);
            continue;
        }

        i2c_packet_t packet;
        if (!ctrl_i2c_packet_decode(rx_buffer, n, &packet)) {
            ctrl_stdio_printf(
                "[MASTER][ERR] Pachet invalid (%u octeți, HEAD=0x%02X)\n",
                static_cast<unsigned>(n),
                static_cast<unsigned>(rx_buffer[0]));
            dd_led_set(ALERT_LED_PIN, LOW);
            vTaskDelayUntil(&last_wake, period);
            continue;
        }

        ctrl_stdio_printf(
            "  Pachet OK — HEAD=0x%02X LEN=%u CS=0x%02X\n",
            static_cast<unsigned>(packet.head),
            static_cast<unsigned>(packet.length),
            static_cast<unsigned>(packet.checksum));

        for (uint8_t i = 0; i < HCSR04_SENSOR_COUNT; ++i) {
            demo_print_distance(i, ctrl_i2c_packet_get_value(&packet, i));
        }

        demo_update_alert(packet);
        vTaskDelayUntil(&last_wake, period);
    }
}

}  // namespace

void setup() {
    ctrl_stdio_init(SERIAL_BAUDRATE);
    print_boot_banner();

    dd_led_init(ALERT_LED_PIN);

    // Slave fără hardware I²C (bus = nullptr) — expune pachetele doar prin
    // `app_i2c_slave_peek_response`.
    app_i2c_slave_init(nullptr, 0, 0, I2C_SLAVE_ADDRESS);

    const BaseType_t ok = xTaskCreate(
        task_demo_master_poller,
        "demo_master",
        TASK_STACK_SIZE,
        nullptr,
        TASK_PRIORITY_POLLER,
        nullptr);

    if (ok != pdPASS) {
        ctrl_stdio_print_text("[DEMO][EROARE] Nu am putut crea task-ul master!\n");
        return;
    }

    ctrl_stdio_printf(
        "[DEMO] gata. Master virtual interoghează la fiecare %u ms.\n",
        static_cast<unsigned>(MASTER_POLL_PERIOD_MS));
}

void loop() {
    vTaskDelete(nullptr);
}

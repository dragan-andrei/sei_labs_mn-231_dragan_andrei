#include "app_i2c_master.h"

#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "configs.h"
#include "ctrl_i2c_packet.h"
#include "ctrl_stdio.h"
#include "dd_led.h"

namespace {

// Trimite comanda READ_ALL și citește răspunsul complet.
// Returnează numărul de octeți primiți (0 în caz de eroare).
size_t request_sensor_packet(uint8_t* buffer, size_t capacity) {
    // Faza 1: notificare comandă (master -> slave)
    Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    Wire.write(static_cast<uint8_t>(I2C_CMD_READ_ALL));
    const uint8_t tx_status = Wire.endTransmission();
    if (tx_status != 0) {
        ctrl_stdio_printf(
            "[MASTER][ERR] endTransmission status=%u (slave indisponibil)\n",
            static_cast<unsigned>(tx_status));
        return 0;
    }

    // Faza 2: cerere date (slave -> master)
    const size_t expected = I2C_PACKET_OVERHEAD +
                            (HCSR04_SENSOR_COUNT * sizeof(uint16_t));
    const size_t received_count = Wire.requestFrom(
        static_cast<int>(I2C_SLAVE_ADDRESS),
        static_cast<int>(expected));

    if (received_count == 0 || received_count > capacity) {
        return 0;
    }

    size_t idx = 0;
    while (Wire.available() > 0 && idx < capacity) {
        buffer[idx++] = static_cast<uint8_t>(Wire.read());
    }
    return idx;
}

void print_distance_line(uint8_t sensor_index, uint16_t cm) {
    if (cm == HCSR04_DISTANCE_ERROR) {
        ctrl_stdio_printf("  Senzor #%u: ---- (timeout / out of range)\n",
                          static_cast<unsigned>(sensor_index + 1));
    } else {
        ctrl_stdio_printf("  Senzor #%u: %u cm\n",
                          static_cast<unsigned>(sensor_index + 1),
                          static_cast<unsigned>(cm));
    }
}

// Comportament adițional: LED-ul de alertă se aprinde dacă oricare dintre
// senzori raportează o distanță sub pragul ALERT_PROXIMITY_CM.
void update_proximity_alert(const i2c_packet_t& packet) {
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

void task_master_poller(void* parameters) {
    (void)parameters;
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(MASTER_POLL_PERIOD_MS);

    uint8_t rx_buffer[I2C_PACKET_MAX_SIZE];

    for (;;) {
        ctrl_stdio_printf(
            "\n[%6lu][MASTER] --- interogare slave 0x%02X ---\n",
            millis(),
            static_cast<unsigned>(I2C_SLAVE_ADDRESS));

        const size_t n = request_sensor_packet(rx_buffer, sizeof(rx_buffer));
        if (n == 0) {
            ctrl_stdio_print_text("[MASTER][ERR] Slave-ul nu a răspuns.\n");
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
            const uint16_t cm = ctrl_i2c_packet_get_value(&packet, i);
            print_distance_line(i, cm);
        }

        update_proximity_alert(packet);
        vTaskDelayUntil(&last_wake, period);
    }
}

}  // namespace

void app_i2c_master_init() {
    dd_led_init(ALERT_LED_PIN);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_BUS_CLOCK_HZ);

    const BaseType_t ok = xTaskCreate(
        task_master_poller,
        "i2c_poller",
        TASK_STACK_SIZE,
        nullptr,
        TASK_PRIORITY_POLLER,
        nullptr);

    if (ok != pdPASS) {
        ctrl_stdio_print_text("[MASTER][EROARE] Nu am putut crea task-ul!\n");
        return;
    }

    ctrl_stdio_printf(
        "[MASTER] gata. Voi interoga 0x%02X la fiecare %u ms.\n",
        static_cast<unsigned>(I2C_SLAVE_ADDRESS),
        static_cast<unsigned>(MASTER_POLL_PERIOD_MS));
}

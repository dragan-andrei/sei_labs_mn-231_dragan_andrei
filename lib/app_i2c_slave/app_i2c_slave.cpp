#include "app_i2c_slave.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "configs.h"
#include "ctrl_i2c_packet.h"
#include "ctrl_stdio.h"
#include "dd_hcsr04.h"

namespace {

// ---- Descriptori senzori ----
const hcsr04_sensor_t g_sensors[HCSR04_SENSOR_COUNT] = {
    { HCSR04_S1_TRIG_PIN, HCSR04_S1_ECHO_PIN },
    { HCSR04_S2_TRIG_PIN, HCSR04_S2_ECHO_PIN },
};

// Ultimele citiri valide de la senzori (protejate de mutex)
uint16_t g_latest_distance_cm[HCSR04_SENSOR_COUNT] = {
    HCSR04_DISTANCE_ERROR,
    HCSR04_DISTANCE_ERROR,
};

// Buffer deja codificat pentru răspuns I²C — pre-calculat pentru a minimiza
// timpul petrecut în callback-ul onRequest.
uint8_t g_response_buffer[I2C_PACKET_MAX_SIZE];
size_t  g_response_length = 0;

SemaphoreHandle_t g_data_mutex = nullptr;
TwoWire*          g_bus        = nullptr;
uint8_t           g_slave_addr = 0;

void on_i2c_receive_event(int byte_count) {
    if (g_bus == nullptr) {
        return;
    }
    // Master-ul trimite 1 octet: codul de comandă. Îl consumăm din FIFO.
    // Momentan e suportat doar I2C_CMD_READ_ALL, dar structura permite extensii.
    while (g_bus->available() > 0) {
        (void)g_bus->read();
    }
    (void)byte_count;
}

void on_i2c_request_event() {
    if (g_bus == nullptr || g_data_mutex == nullptr) {
        return;
    }
    // Pe ESP32 callback-urile Wire rulează într-un task dedicat al driverului
    // I²C, deci putem prelua mutex-ul (timeout 0). Dacă buffer-ul este momentan
    // rescris de task-ul de refresh, renunțăm la răspuns — master-ul va
    // re-interoga în ciclul următor. Astfel evităm pachete corupte.
    if (xSemaphoreTake(g_data_mutex, 0) != pdTRUE) {
        return;
    }
    if (g_response_length > 0) {
        g_bus->write(g_response_buffer, g_response_length);
    }
    xSemaphoreGive(g_data_mutex);
}

void task_sensor_sampler(void* parameters) {
    (void)parameters;
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(SLAVE_SAMPLE_PERIOD_MS);

    for (;;) {
        for (uint8_t i = 0; i < HCSR04_SENSOR_COUNT; ++i) {
            const uint16_t cm = dd_hcsr04_read_cm(&g_sensors[i]);

            if (xSemaphoreTake(g_data_mutex,
                               pdMS_TO_TICKS(MUTEX_BLOCK_TIMEOUT_MS)) == pdTRUE) {
                g_latest_distance_cm[i] = cm;
                xSemaphoreGive(g_data_mutex);
            }
        }
        vTaskDelayUntil(&last_wake, period);
    }
}

void task_buffer_refresh(void* parameters) {
    (void)parameters;
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(SLAVE_BUFFER_REFRESH_MS);

    for (;;) {
        uint16_t snapshot[HCSR04_SENSOR_COUNT];

        if (xSemaphoreTake(g_data_mutex,
                           pdMS_TO_TICKS(MUTEX_BLOCK_TIMEOUT_MS)) == pdTRUE) {
            for (uint8_t i = 0; i < HCSR04_SENSOR_COUNT; ++i) {
                snapshot[i] = g_latest_distance_cm[i];
            }
            xSemaphoreGive(g_data_mutex);
        } else {
            vTaskDelayUntil(&last_wake, period);
            continue;
        }

        uint8_t encoded[I2C_PACKET_MAX_SIZE];
        const size_t written = ctrl_i2c_packet_encode(
            snapshot,
            HCSR04_SENSOR_COUNT,
            encoded,
            sizeof(encoded));

        if (written > 0 &&
            xSemaphoreTake(g_data_mutex,
                           pdMS_TO_TICKS(MUTEX_BLOCK_TIMEOUT_MS)) == pdTRUE) {
            for (size_t i = 0; i < written; ++i) {
                g_response_buffer[i] = encoded[i];
            }
            g_response_length = written;
            xSemaphoreGive(g_data_mutex);

            const unsigned payload_bytes =
                static_cast<unsigned>(written) -
                static_cast<unsigned>(I2C_PACKET_OVERHEAD);

            ctrl_stdio_printf(
                "[%6lu][SLAVE] refresh buffer: S1=%u cm, S2=%u cm "
                "(payload=%u B, total=%u B)\n",
                millis(),
                static_cast<unsigned>(snapshot[0]),
                static_cast<unsigned>(snapshot[1]),
                payload_bytes,
                static_cast<unsigned>(written));
        }

        vTaskDelayUntil(&last_wake, period);
    }
}

}  // namespace

void app_i2c_slave_init(TwoWire* bus,
                        uint8_t sda_pin,
                        uint8_t scl_pin,
                        uint8_t slave_address) {
    for (uint8_t i = 0; i < HCSR04_SENSOR_COUNT; ++i) {
        dd_hcsr04_init(&g_sensors[i]);
    }

    g_data_mutex = xSemaphoreCreateMutex();
    if (g_data_mutex == nullptr) {
        ctrl_stdio_print_text("[SLAVE][EROARE] Nu am putut aloca mutex-ul!\n");
        return;
    }

    g_bus        = bus;
    g_slave_addr = slave_address;

    if (bus != nullptr) {
        bus->begin(slave_address, sda_pin, scl_pin, I2C_BUS_CLOCK_HZ);
        bus->onReceive(on_i2c_receive_event);
        bus->onRequest(on_i2c_request_event);
    }

    const BaseType_t sampler_ok = xTaskCreate(
        task_sensor_sampler,
        "hcsr04_sampler",
        TASK_STACK_SIZE,
        nullptr,
        TASK_PRIORITY_SAMPLER,
        nullptr);

    const BaseType_t buffer_ok = xTaskCreate(
        task_buffer_refresh,
        "i2c_buffer",
        TASK_STACK_SIZE,
        nullptr,
        TASK_PRIORITY_BUFFER,
        nullptr);

    if (sampler_ok != pdPASS || buffer_ok != pdPASS) {
        ctrl_stdio_print_text("[SLAVE][EROARE] Nu am putut crea task-urile!\n");
        return;
    }

    if (bus != nullptr) {
        ctrl_stdio_printf(
            "[SLAVE] gata. Adresa 0x%02X (SDA=%u, SCL=%u), %u senzori HC-SR04.\n",
            static_cast<unsigned>(slave_address),
            static_cast<unsigned>(sda_pin),
            static_cast<unsigned>(scl_pin),
            static_cast<unsigned>(HCSR04_SENSOR_COUNT));
    } else {
        ctrl_stdio_printf(
            "[SLAVE] gata (DEMO / magistrală virtuală), %u senzori HC-SR04.\n",
            static_cast<unsigned>(HCSR04_SENSOR_COUNT));
    }
}

size_t app_i2c_slave_peek_response(uint8_t* out, size_t capacity) {
    if (out == nullptr || g_data_mutex == nullptr) {
        return 0;
    }
    // Timeout generos în context de task (demo-ul rulează la 500 ms — suficient).
    if (xSemaphoreTake(g_data_mutex,
                       pdMS_TO_TICKS(MUTEX_BLOCK_TIMEOUT_MS)) != pdTRUE) {
        return 0;
    }
    size_t n = g_response_length;
    if (n > capacity) {
        n = capacity;
    }
    for (size_t i = 0; i < n; ++i) {
        out[i] = g_response_buffer[i];
    }
    xSemaphoreGive(g_data_mutex);
    return n;
}

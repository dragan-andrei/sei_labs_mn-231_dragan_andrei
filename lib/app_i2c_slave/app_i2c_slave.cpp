#include "app_i2c_slave.h"

#include <Wire.h>
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

// Ultimele citiri valide de la senzori (protejate de mutex-ul de mai jos)
uint16_t g_latest_distance_cm[HCSR04_SENSOR_COUNT] = {
    HCSR04_DISTANCE_ERROR,
    HCSR04_DISTANCE_ERROR,
};

// Buffer deja codificat pentru răspuns I²C — pre-calculat pentru a minimiza
// timpul petrecut în ISR-ul onRequest.
uint8_t g_response_buffer[I2C_PACKET_MAX_SIZE];
size_t  g_response_length = 0;

SemaphoreHandle_t g_data_mutex = nullptr;

void on_i2c_receive_event(int byte_count) {
    // Masterul trimite 1 octet: codul de comandă. Îl consumăm din FIFO-ul Wire
    // (momentan e folosit doar I2C_CMD_READ_ALL, dar structura permite extensii).
    while (Wire.available() > 0) {
        (void)Wire.read();
    }
    (void)byte_count;
}

void on_i2c_request_event() {
    // Pe ESP32 callback-urile Wire sunt invocate dintr-un task dedicat al
    // driverului I²C (nu dintr-un ISR pur), deci este sigur să preluăm
    // mutex-ul cu timeout mic. Dacă buffer-ul este momentan actualizat de
    // task-ul de refresh, renunțăm la răspuns (master-ul va re-interoga).
    if (g_data_mutex == nullptr) {
        return;
    }
    if (xSemaphoreTake(g_data_mutex, 0) != pdTRUE) {
        return;
    }
    if (g_response_length > 0) {
        Wire.write(g_response_buffer, g_response_length);
    }
    xSemaphoreGive(g_data_mutex);
}

// Task: achiziție periodică a distanței de la fiecare senzor.
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

// Task: serializează citirile curente în buffer-ul de răspuns I²C.
// Separarea față de task-ul de achiziție permite ca encoder-ul să ruleze la
// altă frecvență (sau să fie reutilizat pentru alt transport).
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

            ctrl_stdio_printf(
                "[%6lu][SLAVE] refresh buffer: S1=%u cm, S2=%u cm (payload=%u B)\n",
                millis(),
                static_cast<unsigned>(snapshot[0]),
                static_cast<unsigned>(snapshot[1]),
                static_cast<unsigned>(written));
        }

        vTaskDelayUntil(&last_wake, period);
    }
}

}  // namespace

void app_i2c_slave_init() {
    for (uint8_t i = 0; i < HCSR04_SENSOR_COUNT; ++i) {
        dd_hcsr04_init(&g_sensors[i]);
    }

    g_data_mutex = xSemaphoreCreateMutex();
    if (g_data_mutex == nullptr) {
        ctrl_stdio_print_text("[SLAVE][EROARE] Nu am putut aloca mutex-ul!\n");
        return;
    }

    Wire.begin(I2C_SLAVE_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN, I2C_BUS_CLOCK_HZ);
    Wire.onReceive(on_i2c_receive_event);
    Wire.onRequest(on_i2c_request_event);

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

    ctrl_stdio_printf(
        "[SLAVE] gata. Adresa 0x%02X, %u senzori HC-SR04.\n",
        static_cast<unsigned>(I2C_SLAVE_ADDRESS),
        static_cast<unsigned>(HCSR04_SENSOR_COUNT));
}

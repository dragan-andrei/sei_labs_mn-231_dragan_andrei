#include "app_mqtt_node.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>

#include "configs.h"
#include "ctrl_stdio.h"
#include "dd_dht22.h"
#include "dd_led.h"
#include "dd_mqtt.h"
#include "dd_wifi.h"

namespace {

// Stare & statistici
volatile bool     g_led_on        = false;
volatile uint32_t g_stat_samples  = 0;
volatile uint32_t g_stat_published = 0;
volatile uint32_t g_stat_failed_pub = 0;
volatile uint32_t g_stat_cmd_on   = 0;
volatile uint32_t g_stat_cmd_off  = 0;
volatile uint32_t g_stat_cmd_toggle = 0;
volatile uint32_t g_stat_cmd_unknown = 0;

void apply_led(bool on) {
    g_led_on = on;
    if (on) {
        dd_led_on(LED_ACTUATOR_PIN);
    } else {
        dd_led_off(LED_ACTUATOR_PIN);
    }
    // Publicăm STATE-ul efectiv pentru ca dashboard-ul să reflecte starea
    // reală (nu doar comanda).
    dd_mqtt_publish(MQTT_TOPIC_LED_STATE, on ? MQTT_COMMAND_ON : MQTT_COMMAND_OFF);
    ctrl_stdio_printf("[APP] LED -> %s\n", on ? "ON" : "OFF");
}

void handle_led_cmd(const uint8_t* payload, unsigned int length) {
    // Copiem într-un buffer zero-terminat pentru comparație sigură.
    char cmd[16] = {0};
    const unsigned int n =
        (length < sizeof(cmd) - 1) ? length : sizeof(cmd) - 1;
    for (unsigned int i = 0; i < n; ++i) {
        cmd[i] = static_cast<char>(payload[i]);
    }

    if (strcmp(cmd, MQTT_COMMAND_ON) == 0) {
        ++g_stat_cmd_on;
        apply_led(true);
    } else if (strcmp(cmd, MQTT_COMMAND_OFF) == 0) {
        ++g_stat_cmd_off;
        apply_led(false);
    } else if (strcmp(cmd, MQTT_COMMAND_TOGGLE) == 0) {
        ++g_stat_cmd_toggle;
        apply_led(!g_led_on);
    } else {
        ++g_stat_cmd_unknown;
        ctrl_stdio_printf("[APP][WARN] Comandă necunoscută: \"%s\"\n", cmd);
    }
}

void mqtt_on_message(const char* topic,
                     const uint8_t* payload,
                     unsigned int length) {
    ctrl_stdio_printf("[MQTT][ RX ] topic=\"%s\"  len=%u\n",
                      topic,
                      static_cast<unsigned>(length));
    if (strcmp(topic, MQTT_TOPIC_LED_CMD) == 0) {
        handle_led_cmd(payload, length);
    }
}

void ensure_connections() {
    if (!dd_wifi_is_connected()) {
        dd_wifi_connect_blocking(WIFI_SSID, WIFI_PASSWORD,
                                 WIFI_CONNECT_TIMEOUT_MS);
    }
    if (dd_wifi_is_connected() && !dd_mqtt_is_connected()) {
        if (dd_mqtt_connect()) {
            dd_mqtt_subscribe(MQTT_TOPIC_LED_CMD);
            // Republicăm starea curentă pentru ca dashboard-ul să se
            // sincronizeze după reconectare.
            dd_mqtt_publish(MQTT_TOPIC_LED_STATE,
                            g_led_on ? MQTT_COMMAND_ON : MQTT_COMMAND_OFF);
        } else {
            vTaskDelay(pdMS_TO_TICKS(MQTT_RECONNECT_PERIOD_MS));
        }
    }
}

void task_net(void* parameters) {
    (void)parameters;
    for (;;) {
        ensure_connections();
        dd_mqtt_loop();
        vTaskDelay(pdMS_TO_TICKS(NET_LOOP_PERIOD_MS));
    }
}

void publish_sample(const dd_dht22_sample_t& s) {
    char payload[MQTT_MAX_PAYLOAD_BYTES];
    const int n = snprintf(
        payload, sizeof(payload),
        "{\"temperature\":%.2f,\"humidity\":%.2f,\"uptime_ms\":%lu}",
        static_cast<double>(s.temperature_c),
        static_cast<double>(s.humidity_percent),
        static_cast<unsigned long>(millis()));
    if (n <= 0 || static_cast<size_t>(n) >= sizeof(payload)) {
        ++g_stat_failed_pub;
        ctrl_stdio_print_text("[APP][ERR] snprintf payload overflow\n");
        return;
    }
    ctrl_stdio_printf("[APP][ TX ] %s -> %s\n", MQTT_TOPIC_TELEMETRY, payload);
    const bool ok = dd_mqtt_publish(MQTT_TOPIC_TELEMETRY, payload);
    if (ok) {
        ++g_stat_published;
    } else {
        ++g_stat_failed_pub;
        ctrl_stdio_print_text("[APP][ERR] publish eșuat (MQTT indisponibil?)\n");
    }
}

void task_sensor(void* parameters) {
    (void)parameters;
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(SENSOR_SAMPLE_PERIOD_MS);
    for (;;) {
        dd_dht22_sample_t sample;
        dd_dht22_read(&sample);
        ++g_stat_samples;
        if (sample.valid) {
            publish_sample(sample);
        }
        vTaskDelayUntil(&last_wake, period);
    }
}

void task_diag(void* parameters) {
    (void)parameters;
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(DIAG_PERIOD_MS);
    for (;;) {
        vTaskDelayUntil(&last_wake, period);
        ctrl_stdio_printf(
            "[DIAG] wifi=%d mqtt=%d  samples=%lu pub=%lu pub_fail=%lu  "
            "cmd_on=%lu cmd_off=%lu cmd_tog=%lu cmd_?=%lu  led=%s\n",
            dd_wifi_is_connected() ? 1 : 0,
            dd_mqtt_is_connected() ? 1 : 0,
            static_cast<unsigned long>(g_stat_samples),
            static_cast<unsigned long>(g_stat_published),
            static_cast<unsigned long>(g_stat_failed_pub),
            static_cast<unsigned long>(g_stat_cmd_on),
            static_cast<unsigned long>(g_stat_cmd_off),
            static_cast<unsigned long>(g_stat_cmd_toggle),
            static_cast<unsigned long>(g_stat_cmd_unknown),
            g_led_on ? "ON" : "OFF");
    }
}

}  // namespace

void app_mqtt_node_start() {
    // 1. Hardware local
    dd_led_init(LED_ACTUATOR_PIN);
    dd_dht22_init(DHT22_DATA_PIN, DHT22_SENSOR_TYPE);

    // 2. Rețea
    dd_wifi_init();
    dd_wifi_connect_blocking(WIFI_SSID, WIFI_PASSWORD,
                             WIFI_CONNECT_TIMEOUT_MS);

    dd_mqtt_init(MQTT_BROKER_HOST, MQTT_BROKER_PORT, mqtt_on_message);
    if (dd_wifi_is_connected() && dd_mqtt_connect()) {
        dd_mqtt_subscribe(MQTT_TOPIC_LED_CMD);
        dd_mqtt_publish(MQTT_TOPIC_LED_STATE, MQTT_COMMAND_OFF);
    }

    // 3. Task-uri
    xTaskCreate(task_net,    "net",    TASK_STACK_SIZE, nullptr,
                TASK_PRIORITY_NET,    nullptr);
    xTaskCreate(task_sensor, "sensor", TASK_STACK_SIZE, nullptr,
                TASK_PRIORITY_SENSOR, nullptr);
    xTaskCreate(task_diag,   "diag",   TASK_STACK_SIZE, nullptr,
                TASK_PRIORITY_DIAG,   nullptr);
    ctrl_stdio_print_text("[APP] Toate task-urile FreeRTOS active.\n");
}

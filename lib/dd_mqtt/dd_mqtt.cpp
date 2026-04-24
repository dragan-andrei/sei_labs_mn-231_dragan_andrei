#include "dd_mqtt.h"

#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include "configs.h"
#include "ctrl_stdio.h"

namespace {

WiFiClient        g_net_client;
PubSubClient      g_mqtt(g_net_client);
dd_mqtt_message_cb g_user_cb = nullptr;
char              g_last_client_id[48] = {0};

void internal_callback(char* topic, uint8_t* payload, unsigned int length) {
    if (g_user_cb != nullptr) {
        g_user_cb(topic, payload, length);
    }
}

}  // namespace

void dd_mqtt_init(const char* broker_host,
                  uint16_t    broker_port,
                  dd_mqtt_message_cb on_message) {
    g_user_cb = on_message;
    g_mqtt.setServer(broker_host, broker_port);
    g_mqtt.setKeepAlive(MQTT_KEEPALIVE_SECONDS);
    g_mqtt.setBufferSize(MQTT_MAX_PAYLOAD_BYTES + MQTT_MAX_TOPIC_BYTES + 32);
    g_mqtt.setCallback(internal_callback);
    ctrl_stdio_printf("[MQTT] broker=%s:%u, keepalive=%u s\n",
                      broker_host,
                      static_cast<unsigned>(broker_port),
                      static_cast<unsigned>(MQTT_KEEPALIVE_SECONDS));
}

bool dd_mqtt_connect() {
    if (g_mqtt.connected()) {
        return true;
    }
    // clientId aleatoriu ca să nu fim dați afară de alt ESP32 cu același id
    // pe brokerul public.
    snprintf(g_last_client_id,
             sizeof(g_last_client_id),
             "%s%04lx",
             MQTT_CLIENT_ID_PREFIX,
             static_cast<unsigned long>(random(0, 0xFFFF)));

    ctrl_stdio_printf("[MQTT] Conectare, clientId=\"%s\" ...\n",
                      g_last_client_id);
    const bool ok = g_mqtt.connect(g_last_client_id);
    if (!ok) {
        ctrl_stdio_printf("[MQTT][EROARE] state=%d\n",
                          static_cast<int>(g_mqtt.state()));
        return false;
    }
    ctrl_stdio_print_text("[MQTT] Conectat la broker\n");
    return true;
}

void dd_mqtt_loop() {
    g_mqtt.loop();
}

bool dd_mqtt_publish(const char* topic, const char* payload) {
    if (topic == nullptr || payload == nullptr) {
        return false;
    }
    if (!g_mqtt.connected()) {
        return false;
    }
    return g_mqtt.publish(topic, payload);
}

bool dd_mqtt_publish_raw(const char* topic,
                         const uint8_t* payload,
                         size_t length) {
    if (topic == nullptr || payload == nullptr) {
        return false;
    }
    if (!g_mqtt.connected()) {
        return false;
    }
    return g_mqtt.publish(topic,
                          payload,
                          static_cast<unsigned int>(length),
                          false);
}

bool dd_mqtt_subscribe(const char* topic) {
    if (topic == nullptr) {
        return false;
    }
    if (!g_mqtt.connected()) {
        return false;
    }
    const bool ok = g_mqtt.subscribe(topic);
    ctrl_stdio_printf("[MQTT] Subscribe topic=\"%s\" => %s\n",
                      topic,
                      ok ? "OK" : "EROARE");
    return ok;
}

bool dd_mqtt_is_connected() {
    return g_mqtt.connected();
}

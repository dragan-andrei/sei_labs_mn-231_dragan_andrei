#ifndef CONFIGS_H
#define CONFIGS_H

#include <Arduino.h>
#include <stdint.h>

// ============================================================================
//  LAB 7.3 — Configurații centralizate (no magic numbers)
// ============================================================================
//  Orice modificare de pini, topicuri, credentials, ritm de publicare etc.
//  se face STRICT într-un singur loc — aici. Celelalte module nu folosesc
//  constante hard-codate.
// ============================================================================

// ---------- STDIO (debug peste USB) -----------------------------------------
#define SERIAL_BAUDRATE                 115200UL

// ---------- Wi-Fi -----------------------------------------------------------
// Pentru Wokwi se folosește rețeaua "Wokwi-GUEST", fără parolă. Pentru flash
// fizic, aceste două constante se înlocuiesc cu SSID-ul și parola reală ale
// rețelei studentului / laboratorului. Nu se comit credentials reale.
#define WIFI_SSID                       "Wokwi-GUEST"
#define WIFI_PASSWORD                   ""
#define WIFI_CONNECT_TIMEOUT_MS         30000UL
#define WIFI_RETRY_PERIOD_MS            500UL

// ---------- MQTT broker -----------------------------------------------------
// Broker public pentru Lab 7.3 (fără autentificare, port TCP 1883).
// Alternativă: "test.mosquitto.org", "broker.emqx.io".
#define MQTT_BROKER_HOST                "broker.hivemq.com"
#define MQTT_BROKER_PORT                1883
#define MQTT_KEEPALIVE_SECONDS          30
#define MQTT_RECONNECT_PERIOD_MS        5000UL
#define MQTT_CLIENT_ID_PREFIX           "esp32-utm-sei-lab73-"

// Topicuri. Spațiul de nume include "utm/sei/lab73" + username pentru a evita
// coliziunile pe brokerul public cu alți studenți.
#define MQTT_TOPIC_TELEMETRY            "utm/sei/lab73/dragan/telemetry"
#define MQTT_TOPIC_LED_CMD              "utm/sei/lab73/dragan/led/cmd"
#define MQTT_TOPIC_LED_STATE            "utm/sei/lab73/dragan/led/state"

#define MQTT_COMMAND_ON                 "ON"
#define MQTT_COMMAND_OFF                "OFF"
#define MQTT_COMMAND_TOGGLE             "TOGGLE"

// Payload limits (folosite pentru dimensionarea buffer-elor interne).
#define MQTT_MAX_PAYLOAD_BYTES          256
#define MQTT_MAX_TOPIC_BYTES            128

// ---------- Pinii hardware --------------------------------------------------
#define DHT22_DATA_PIN                  4
#define DHT22_SENSOR_TYPE               22   // DHT22 (AM2302)
#define LED_ACTUATOR_PIN                2    // LED-ul onboard al DevKit-ului

// ---------- FreeRTOS --------------------------------------------------------
#define TASK_STACK_SIZE                 8192

#define TASK_PRIORITY_NET               3    // menține WiFi+MQTT vii
#define TASK_PRIORITY_SENSOR            2
#define TASK_PRIORITY_DIAG              1

#define SENSOR_SAMPLE_PERIOD_MS         5000UL
#define NET_LOOP_PERIOD_MS              50UL    // poll MQTT / WiFi
#define DIAG_PERIOD_MS                  10000UL

// ---------- DHT22 read guard ------------------------------------------------
// Valori considerate "rezonabile" — orice în afară => citire invalidă
// (NaN sau senzor defect).
#define DHT22_MIN_TEMPERATURE_C         (-40.0f)
#define DHT22_MAX_TEMPERATURE_C         (85.0f)
#define DHT22_MIN_HUMIDITY_PERCENT      (0.0f)
#define DHT22_MAX_HUMIDITY_PERCENT      (100.0f)

#endif // CONFIGS_H

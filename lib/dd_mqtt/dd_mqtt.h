#ifndef DD_MQTT_H
#define DD_MQTT_H

#include <stddef.h>
#include <stdint.h>

// ============================================================================
//  dd_mqtt — HAL peste biblioteca PubSubClient.
// ============================================================================
//  Codul aplicației nu include direct PubSubClient / WiFiClient; doar
//  interfața asta. Permite încapsulare curată și — dacă e nevoie — schimbarea
//  ulterioară a bibliotecii MQTT fără atingerea logicii de nivel superior.

// Prototipul callback-ului apelat la primirea unui mesaj.
//  - `topic`   : null-terminated, copie ownership deținută de dd_mqtt
//  - `payload` : buffer raw (NU este null-terminated)
//  - `length`  : dimensiunea payload-ului în bytes
typedef void (*dd_mqtt_message_cb)(const char* topic,
                                   const uint8_t* payload,
                                   unsigned int length);

// Inițializează clientul MQTT cu host-ul și portul brokerului. Trebuie apelat
// DUPĂ ce WiFi-ul e conectat.
void dd_mqtt_init(const char* broker_host,
                  uint16_t    broker_port,
                  dd_mqtt_message_cb on_message);

// Încercare blocantă de conectare (până la un ciclu de retry de către caller).
// Generează un clientId unic derivat din random, ca să nu se ciocnească cu
// alte plăci pe brokerul public.
bool dd_mqtt_connect();

// Rulează bucla internă de IO (heartbeat keepalive + procesare mesaje primite).
// Trebuie apelată periodic din task-ul de rețea.
void dd_mqtt_loop();

// Publish pe `topic`. `payload` poate fi un șir null-terminated sau orice
// buffer raw (când se folosește `length` != 0).
bool dd_mqtt_publish(const char* topic, const char* payload);
bool dd_mqtt_publish_raw(const char* topic,
                         const uint8_t* payload,
                         size_t length);

// Abonare la un topic (poate conține wildcard-urile MQTT standard).
bool dd_mqtt_subscribe(const char* topic);

// Returnează `true` dacă sesiunea MQTT e activă cu brokerul.
bool dd_mqtt_is_connected();

#endif // DD_MQTT_H

#ifndef DD_WIFI_H
#define DD_WIFI_H

#include <Arduino.h>
#include <stddef.h>

// ============================================================================
//  dd_wifi — HAL peste biblioteca WiFi a Arduino-ESP32.
// ============================================================================
//  Expune funcții simple de inițializare, verificare și reconectare, astfel
//  încât modulele de sus (dd_mqtt, app_*) să nu depindă direct de
//  `WiFi.begin/status/reconnect` — doar de interfața asta.

// Inițializează stiva WiFi în modul STA (station) și dezactivează sleep-ul
// de putere ca să avem latență minimă la traficul MQTT.
void dd_wifi_init();

// Încearcă conexiunea la rețeaua configurată. Returnează `true` dacă s-a
// asociat înainte de `timeout_ms`.
bool dd_wifi_connect_blocking(const char* ssid,
                              const char* password,
                              unsigned long timeout_ms);

// Returnează `true` dacă placa are un IP valid în rețea.
bool dd_wifi_is_connected();

// Tipărește pe STDIO starea curentă (IP, RSSI).
void dd_wifi_print_status();

#endif // DD_WIFI_H

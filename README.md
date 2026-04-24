# Lab 7.3 — Comunicare Internet: MQTT cu broker Cloud

> Stud. MN-231 — Dragan Andrei

Un **nod IoT bidirecțional** bazat pe ESP32 care:

1. **Publică telemetrie** (temperatură + umiditate de la DHT22) pe un
   broker MQTT public — payload JSON la fiecare 5 s.
2. **Ascultă comenzi** pe un topic dedicat pentru a controla un actuator
   (LED pe GPIO 2) — `ON` / `OFF` / `TOGGLE`.
3. **Publică starea reală** a actuatorului înapoi pe broker, așa încât un
   dashboard (ThingsBoard, HiveMQ Insights, MQTT Explorer) să reflecte
   consistent realitatea placii.

Tot codul rulează peste **FreeRTOS**, cu task-uri separate pentru rețea,
senzor și diagnosticare. Structura este aceeași ca în lab 7.1 / 7.2 —
`dd_*` / `ctrl_*` / `app_*`, toate constantele centralizate în
`include/configs.h`, zero magic numbers.

## 1. Arhitectura sistemului

```
                         ┌──────────────────────┐
                         │ Dashboard Cloud      │
                         │ (HiveMQ / ThingsBoard│
                         │  / MQTT Explorer)    │
                         └──────────┬───────────┘
                                    │ MQTT pub/sub TCP 1883
                                    │
                             broker.hivemq.com
                                    │
                ┌───────────────────┴────────────────────┐
                │ ESP32 (esp32dev)                       │
                │ ┌──────────────┐ ┌──────────────┐      │
                │ │ task_net     │ │ task_sensor  │      │
                │ │ (prio 3)     │ │ (prio 2)     │      │
                │ │ WiFi + MQTT  │ │ DHT22 -> JSON│      │
                │ │ reconnect    │ │  publish 5 s │      │
                │ └──────┬───────┘ └──────┬───────┘      │
                │        │                │              │
                │   dd_wifi, dd_mqtt    dd_dht22         │
                │        │                │              │
                │        ▼                ▼              │
                │ callback MQTT --> apply_led() -> dd_led│
                │ (ON / OFF / TOGGLE)                    │
                │ task_diag (prio 1) — stats             │
                └────────────────────────────────────────┘
                       │                 │
                       │ GPIO 4          │ GPIO 2 + R 220 Ω
                       ▼                 ▼
                     DHT22              LED
```

## 2. Topicuri MQTT

| Topic                                           | Direcție     | Payload                                                                 |
|-------------------------------------------------|--------------|-------------------------------------------------------------------------|
| `utm/sei/lab73/dragan/telemetry`                | ESP32 → Cloud| JSON: `{"temperature":24.00,"humidity":55.00,"uptime_ms":1234567}`      |
| `utm/sei/lab73/dragan/led/cmd`                  | Cloud → ESP32| `ON`, `OFF`, `TOGGLE`                                                    |
| `utm/sei/lab73/dragan/led/state`                | ESP32 → Cloud| `ON` / `OFF` — starea reală, publicată după fiecare schimbare          |

Space-ul de nume `utm/sei/lab73/dragan/...` previne coliziunile pe broker-ul
public — orice modificare se face dintr-un singur loc, `include/configs.h`.

## 3. Structura proiectului

```
include/configs.h           — WiFi/MQTT/topic/pins/task (toate no-magic-numbers)
lib/
 ├─ ctrl_stdio/             — serial debug (Serial0 @ 115200)
 ├─ dd_led/                 — driver LED
 ├─ dd_wifi/                — HAL WiFi.h (STA, connect blocking, status)
 ├─ dd_mqtt/                — HAL PubSubClient (connect, pub, sub, loop)
 ├─ dd_dht22/               — HAL DHT sensor library + validare interval
 └─ app_mqtt_node/          — task_net + task_sensor + task_diag + callback
src/
 └─ main.cpp                — banner + app_mqtt_node_start()
```

## 4. Task-urile FreeRTOS

| Task         | Prioritate | Rol                                                                             |
|--------------|------------|---------------------------------------------------------------------------------|
| `task_net`   | 3          | Menține WiFi+MQTT active, reconnect automat, rulează `dd_mqtt_loop()` la 50 ms  |
| `task_sensor`| 2          | La fiecare 5 s (`vTaskDelayUntil`) citește DHT22 și publică JSON pe TELEMETRY   |
| `task_diag`  | 1          | La fiecare 10 s tipărește statistici complete pe STDIO                          |

Callback-ul MQTT (`mqtt_on_message`) rulează în contextul task-ului de rețea
atunci când brokerul ne trimite un mesaj — e decuplat de buclă prin
PubSubClient, așa că task_sensor nu se blochează niciodată.

## 5. Cablajul electric (Wokwi `diagram.json`)

```
   ESP32 3V3   ─── DHT22 VCC
   ESP32 GND.1 ─── DHT22 GND
   ESP32 GPIO 4─── DHT22 SDA (data)
   ESP32 GPIO 2─── R 220 Ω ─── LED anod
   ESP32 GND.2 ─── LED catod
```

## 6. Configurare broker / dashboard

### 6.1. MQTT Explorer (debug rapid)

- Host: `broker.hivemq.com`, port `1883`, fără autentificare.
- Subscribe pe `utm/sei/lab73/dragan/#` — vezi imediat telemetria + state-ul
  LED-ului.
- Publish pe `utm/sei/lab73/dragan/led/cmd` cu payload `ON` / `OFF` / `TOGGLE`.

### 6.2. HiveMQ Web Client

<https://www.hivemq.com/demos/websocket-client/> — se conectează automat la
brokerul public. Folosește aceleași topicuri ca mai sus.

### 6.3. ThingsBoard (dashboard full)

1. Cont gratuit pe <https://demo.thingsboard.io>.
2. Creează un **device** de tip *Default* (notează `Access Token`).
3. Înlocuiește în `configs.h`:
   - `MQTT_BROKER_HOST` = `"demo.thingsboard.io"`
   - `MQTT_BROKER_PORT` = `1883`
   - clientId / user = `Access Token` (adaptare în `dd_mqtt_connect`)
4. Publică pe `v1/devices/me/telemetry` și subscribe la
   `v1/devices/me/rpc/request/+` pentru comenzi — echivalent pe ThingsBoard
   cu topicurile noastre din `configs.h`.

## 7. Build & Run

### 7.1. Wokwi (simulare)

```bash
pio run                # build
# apoi „Start simulation" în VSCode Wokwi
```

Rețeaua Wi-Fi simulată este `Wokwi-GUEST` (fără parolă) — DHCP automat.

### 7.2. Hardware fizic

```bash
# 1. Editează include/configs.h:
#    - WIFI_SSID     = "numele retelei tale"
#    - WIFI_PASSWORD = "parola"
#    - (opțional) MQTT_TOPIC_*  cu username-ul tău
# 2. Build + flash:
pio run -t upload
pio device monitor
```

## 8. Mapare la cerințele lucrării

| Cerință                                                      | Implementare |
|--------------------------------------------------------------|--------------|
| nota 5 — simpla aplicație de comunicare                      | `app_mqtt_node` + `dd_mqtt` + `dd_wifi` funcțional end-to-end |
| +1 — implementare modulară                                   | `dd_*` / `ctrl_*` / `app_*`, `configs.h`, un modul per responsabilitate |
| +1 — ESP32 trimite date către broker                         | `task_sensor` → `dd_mqtt_publish(TOPIC_TELEMETRY, json)` la 5 s |
| +1 — ESP32 primește date de la server                        | `dd_mqtt_subscribe(TOPIC_LED_CMD)` + callback → `apply_led(on)` |
| +1 — datele vizualizate și controlate de la dashboard        | Compatibil cu HiveMQ WebSocket / MQTT Explorer / ThingsBoard (secțiunea 6) |
| +1 — probă de implementare fizică                            | **Necesită demonstrare pe placă reală** (flash + monitor + test) |

## 9. Troubleshooting rapid

| Problemă în terminal                                   | Cauza probabilă                                     | Soluție                                                            |
|---------------------------------------------------------|-----------------------------------------------------|--------------------------------------------------------------------|
| `[WIFI][EROARE] Timeout fără asociere`                  | SSID / parolă greșite sau rețea în afara razei     | Verifică `WIFI_SSID` și `WIFI_PASSWORD` din `configs.h`            |
| `[MQTT][EROARE] state=-2`                               | Socket TCP nu se deschide (fără internet)           | Verifică că placa primește IP (`[WIFI] OK IP=…`)                    |
| `[DHT22][WARN] Citire NaN`                              | Cablaj DHT22 greșit sau fără pull-up                | În Wokwi: senzorul are pull-up intern. Fizic: 10 kΩ SDA↔VCC        |
| Dashboard nu primește date                              | Alt topic sau alt broker                            | Subscribe pe `utm/sei/lab73/dragan/#` pe broker-ul din `configs.h` |

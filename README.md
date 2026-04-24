# Lab 7.1 — Comunicații cu module periferice: I²C (Protocol HW)

**Curs:** Sisteme Electronice Încorporate (SEI), grupa MN-231
**Student:** Dragan Andrei
**Placa:** ESP32 DevKit-C v4 (× 2)
**Framework:** Arduino + FreeRTOS, PlatformIO

---

## 1. Scop

Aplicație dual-MCU în care un microcontroler **Slave** colectează distanțe reale
de la mai mulți senzori ultrasonici HC-SR04, iar un microcontroler **Master** le
interoghează periodic prin magistrala **I²C** folosind un protocol binar
structurat (`HEAD / LENGTH / PAYLOAD / CHECKSUM`). Toată logica este organizată
în task-uri FreeRTOS, iar resursele partajate sunt protejate cu mutex.

## 2. Arhitectura sistemului

```
          +---------------------+                   +---------------------+
          |      ESP32 SLAVE    |                   |      ESP32 MASTER   |
          |  (0x42 pe I2C bus)  |<===== I²C =======>|                     |
          |                     |    SDA / SCL +    |                     |
          |  +---------------+  |     GND comun     |  +---------------+  |
HC-SR04 --+->| dd_hcsr04     |  |                   |  | app_i2c_master|->+-- LED alertă
HC-SR04 --+->|  (HAL senzor) |  |                   |  |  (poller RTOS)|  |   proximitate
          |  +-------+-------+  |                   |  +-------+-------+  |   (GPIO 2)
          |          |          |                   |          |          |
          |  +-------v-------+  |                   |  +-------v-------+  |
          |  | app_i2c_slave |  |                   |  | ctrl_i2c_pack.|  |
          |  |  task sampler |  |                   |  | decode + CS   |  |
          |  |  task refresh |  |                   |  +-------+-------+  |
          |  |  mutex shared |  |                   |          |          |
          |  +-------+-------+  |                   |  +-------v-------+  |
          |          |          |                   |  | ctrl_stdio    |  |
          |  +-------v-------+  |                   |  | (Serial)      |  |
          |  | ctrl_i2c_pack.|  |                   |  +---------------+  |
          |  | encode + CS   |  |                   |                     |
          |  +---------------+  |                   |                     |
          +----------+----------+                   +----------+----------+
                     |                                         |
                     +-------------------+---------------------+
                                         |
                                  include/configs.h
                        (pini, adresă I²C, timpi, praguri)
```

### 2.1 Protocolul de aplicație peste I²C

Secvența unei interogări (poll-cycle):

```
MASTER → SLAVE :  [CMD=0xA1]                     // "citește toți senzorii"
MASTER ← SLAVE :  [HEAD=0xAA][LEN=N*2][PAYLOAD][CHECKSUM]
                                  └──── N × uint16 big-endian ────┘
CHECKSUM = XOR(HEAD, LEN, PAYLOAD[0..LEN-1])
```

- `HEAD = 0xAA` — sincronizare / sanity check.
- `LEN` — lungimea payload-ului în octeți (2 × numărul de senzori).
- `PAYLOAD` — câte un `uint16_t` big-endian pentru fiecare senzor; valoarea
  `0xFFFF` înseamnă citire invalidă / timeout HC-SR04.
- `CHECKSUM` — XOR peste `HEAD || LEN || PAYLOAD`; master-ul îl recalculează și
  respinge pachetele corupte.

### 2.2 Arhitectura FreeRTOS

| MCU    | Task             | Perioada | Prioritate | Rol                                          |
|--------|------------------|----------|------------|----------------------------------------------|
| Slave  | `hcsr04_sampler` | 250 ms   | 2          | Declanșează senzorii, actualizează cache-ul  |
| Slave  | `i2c_buffer`     | 250 ms   | 1          | Serializează cache-ul în buffer-ul de răspuns|
| Slave  | *Wire onRequest* | eveniment| —          | Trimite buffer-ul pe magistrală (non-blocant)|
| Master | `i2c_poller`     | 500 ms   | 2          | Interoghează slave-ul, parsează și afișează  |

Buffer-ul de răspuns (`g_response_buffer`) și cache-ul senzorilor
(`g_latest_distance_cm`) sunt protejate de un `SemaphoreHandle_t` (mutex).
Callback-ul `onRequest` încearcă să preia mutex-ul cu timeout zero — dacă nu-l
obține, omite răspunsul și master-ul va re-interoga în ciclul următor (evită
corupția pachetului).

## 3. Structura proiectului

```
include/
  configs.h                   # Toate macro-urile (pini, adrese, timpi, praguri)
lib/
  dd_hcsr04/                  # HAL senzor ultrasonic (trigger + pulseIn cu timeout)
  dd_led/                     # HAL LED (pentru alerta de proximitate)
  ctrl_stdio/                 # Wrapper Serial / printf
  ctrl_i2c_packet/            # Encode / decode / checksum protocol binar
  app_i2c_slave/              # Task-uri FreeRTOS + Wire onRequest/onReceive
  app_i2c_master/             # Task FreeRTOS poller + afișare
src/
  main_master.cpp             # Entry point pentru env esp32_master
  main_slave.cpp              # Entry point pentru env esp32_slave
platformio.ini                # Două environment-uri selectate prin build_src_filter
diagram.json                  # Circuit Wokwi (2 × ESP32 + 2 × HC-SR04)
wokwi.toml                    # Target implicit pentru simulator
```

Principii respectate:

- **No magic numbers** — toate constantele (pini, timpi, adresă I²C, pragul de
  alertă, dimensiunile pachetului) trăiesc în `include/configs.h`.
- **HAL clar** — modulele `dd_*` ating direct hardware-ul, `ctrl_*` se ocupă de
  logica de protocol / afișare, `app_*` orchestrează task-urile FreeRTOS.
- **Izolare stricte** — `app_i2c_slave` nu știe nimic despre HC-SR04 decât prin
  driver-ul `dd_hcsr04`; invers, `dd_hcsr04` nu știe că datele lui ajung pe I²C.

## 4. Schema electrică

### 4.1 Pini utilizați

| Semnal                | Slave (GPIO) | Master (GPIO) | Notă                                   |
|-----------------------|--------------|---------------|----------------------------------------|
| I²C SDA               | 21           | 21            | pull-up 4.7 kΩ → 3V3                   |
| I²C SCL               | 22           | 22            | pull-up 4.7 kΩ → 3V3                   |
| HC-SR04 #1 TRIG       | 5            | —             |                                        |
| HC-SR04 #1 ECHO       | 18           | —             | divizor recomandat pentru 5V→3V3       |
| HC-SR04 #2 TRIG       | 19           | —             |                                        |
| HC-SR04 #2 ECHO       | 23           | —             |                                        |
| LED alertă proximitate| —            | 2             | Rezistor 220 Ω în serie                |
| GND comun             | GND          | GND           | Obligatoriu legat                       |

### 4.2 Observații critice

- **Masa comună**: ambele plăci, senzorii și LED-ul trebuie să împartă același
  GND, altfel nivelurile logice nu sunt interpretate corect.
- **Pull-up I²C**: deoarece pinii SDA/SCL sunt open-drain, rezistențele externe
  de 4.7 kΩ spre 3V3 sunt necesare pentru stabilitate la 100 kHz.
- **HC-SR04 la 5V**: senzorul răspunde cu 5V pe ECHO; pentru o placă ESP32 se
  recomandă un divizor rezistiv (sau un level shifter) pe linia ECHO, mai ales
  la deployment pe placă reală, pentru a nu stresa GPIO-ul de 3V3.

## 5. Comportamentul adițional

Masterul aprinde **LED-ul de alertă** (GPIO 2) de îndată ce oricare senzor
raportează o distanță sub `ALERT_PROXIMITY_CM = 15 cm`. LED-ul se stinge
automat când obiectul se îndepărtează sau când slave-ul nu mai răspunde, iar
pe consola serială apare linia `>>> ALERTA PROXIMITATE: obiect sub prag! <<<`.

## 6. Build & flash

```bash
# Compilare ambele imagini (implicit)
pio run

# Flash separat pe fiecare placă (cablu USB pe rând)
pio run -e esp32_master -t upload
pio run -e esp32_slave  -t upload

# Serial monitor (master – afișează distanțele și pachetele valide)
pio device monitor -e esp32_master
```

Pentru simularea în Wokwi se compilează default `esp32_slave` și diagrama
furnizează două plăci ESP32 conectate pe I²C.

## 7. Mapare la cerințele lucrării

| Cerință                                                      | Implementare                                                        |
|--------------------------------------------------------------|---------------------------------------------------------------------|
| Două MCU-uri comunicând prin I²C (master ↔ slave)            | `app_i2c_master`, `app_i2c_slave`, configurate pe adresa `0x42`     |
| Slave integrează senzor(i) ultrasonici HC-SR04               | `dd_hcsr04` + 2 senzori descriși în `configs.h`                     |
| Minim două task-uri FreeRTOS pe slave (achiziție + buffer)   | `hcsr04_sampler` și `i2c_buffer` + mutex                             |
| Pachet structurat HEAD / LENGTH / PAYLOAD / CHECKSUM         | `ctrl_i2c_packet` (encode + decode cu XOR checksum)                 |
| Master interoghează periodic și afișează pe STDIO            | `task_master_poller` + `ctrl_stdio` (500 ms)                        |
| Arhitectură modulară (HAL / ctrl / app) fără magic numbers   | Structura `dd_* / ctrl_* / app_*` + `include/configs.h`             |
| Comportament adițional improvizat                             | LED de alertă de proximitate pe master (`ALERT_PROXIMITY_CM`)        |

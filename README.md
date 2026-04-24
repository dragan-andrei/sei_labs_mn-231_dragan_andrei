# Lab 7.2 — Comunicații între dispozitive: protocol logic binar pe UART

> Stud. MN-231 — Dragan Andrei

Două microcontrolere ESP32 comunică printr-un UART hardware dedicat
(`Serial2`), folosind un **protocol logic binar** cu cadre de forma
`[STX][ID][SEQ][CMD][LEN][PAYLOAD][CHECKSUM][ETX]`. Toată aplicația este
construită peste **FreeRTOS**, recepția e decuplată de procesare printr-un
**FIFO** de cadre, iar master-ul are mecanism complet de **ACK/NACK cu
retransmitere** (comportament adițional pentru pontajul maxim).

## 1. Arhitectura sistemului

```
 ┌───────────────────────────┐                 ┌────────────────────────────┐
 │          MASTER           │                 │           SLAVE            │
 │       (ESP32 #1)          │  TX17 ──► RX16  │        (ESP32 #2)          │
 │ ──────────────────────    │ ◄── RX16   TX17 │ ──────────────────────     │
 │  task_master_tx ─► encode │                 │ task_uart_rx ─► FIFO (8)   │
 │       ▲                   │        GND ━━━━ │       │                    │
 │       │inflight_mutex     │                 │       ▼                    │
 │  task_master_rx ◄─ decode │                 │ task_packet_processor      │
 │  (ACK/NACK, retry)        │                 │  (decode, dedup, răspund)  │
 │  task_diag (stats)        │                 │ task_diag (stats)          │
 └───────────────────────────┘                 └────────────────────────────┘
          STDIO: Serial0 (USB, 115200)                STDIO: Serial0 (USB, 115200)
          Trafic:Serial2 binar    9600 8N1            Trafic: Serial2 binar 9600 8N1
```

## 2. Structura cadrului binar

```
 ┌──────┬────┬─────┬─────┬─────┬───────────────────┬──────────┬──────┐
 │ STX  │ ID │ SEQ │ CMD │ LEN │      PAYLOAD      │ CHECKSUM │ ETX  │
 │ 0x02 │ 1B │  1B │  1B │  1B │  0..16 B (LEN)    │   1B XOR │ 0x03 │
 └──────┴────┴─────┴─────┴─────┴───────────────────┴──────────┴──────┘
```

- **CHECKSUM** = XOR peste `ID ^ SEQ ^ CMD ^ LEN ^ PAYLOAD[0..LEN-1]`
- **ID**: `0x01` = master, `0x02` = slave, `0xFF` = broadcast
- **SEQ**: contor 8-bit pe master (wraparound); folosit pentru dedup pe slave
- **CMD-uri definite** (vezi `include/configs.h`):

  | Cod | Nume           | Direcția       | Payload                                 |
  |-----|----------------|----------------|-----------------------------------------|
  | 0x10| `CMD_PING`     | master → slave | gol                                     |
  | 0x11| `CMD_PONG`     | slave → master | 4 B BE = uptime slave (ms)              |
  | 0x20| `CMD_GET_COUNTER` | master → slave | gol                                  |
  | 0x21| `CMD_COUNTER_VALUE` | slave → master | 4 B BE = nr. pachete acceptate     |
  | 0x30| `CMD_ACK`      | ambele         | gol                                     |
  | 0x31| `CMD_NACK`     | slave → master | 2 B: `[requester_id][reason]`           |

## 3. Structura proiectului (modulară, pe trei straturi)

```
include/configs.h          — constantele globale, toate no-magic-numbers
lib/
 ├─ ctrl_stdio/            — wrapper peste Serial0 (debug)
 ├─ dd_led/                — driver LED generic (reutilizat)
 ├─ dd_uart/               — HAL UART + parser cu state machine per cadru
 ├─ ctrl_bin_packet/       — encode / decode / XOR checksum / denumiri CMD
 ├─ app_uart_slave/        — task RX + FIFO + procesare + NACK + diag
 └─ app_uart_master/       — task TX + task RX + retransmitere + diag
src/
 ├─ main_master.cpp        — target esp32_master (flash pe placa fizică #1)
 ├─ main_slave.cpp         — target esp32_slave  (flash pe placa fizică #2)
 └─ main_demo.cpp          — target esp32_demo   (Wokwi, UART virtual)
```

## 4. Task-urile FreeRTOS

| Task                    | Prioritate | Rol                                                                |
|-------------------------|------------|--------------------------------------------------------------------|
| `uart_rx` (master+slave)| 3 (cea mai mare) | Golește UART-ul HW, hrănește parser-ul, validează `[STX..ETX]` |
| `pkt_proc` (slave)      | 2          | Preia cadre din FIFO, decodează, aplică filtrul pe ID + dedup, răspunde |
| `uart_tx` (master)      | 2          | Trimite periodic cereri `PING`/`GET_COUNTER` cu SEQ crescător      |
| `master_diag` / `slave_diag` | 1     | Tipărește periodic statistici (tx / ok / retries / fail / nack)    |

Sincronizare:
- **FIFO** — `xQueueCreate(SLAVE_RX_FIFO_DEPTH, sizeof(fifo_slot_t))` decuplează
  RX-ul fizic de procesarea pachetelor.
- **Mutex `g_inflight_mutex`** (master) — protejează starea cererii în zbor
  (SEQ așteptat, deadline, număr retransmisii).

## 5. Comportament adițional (bonus pontaj)

- **ACK/NACK explicit**: slave-ul răspunde cu `CMD_NACK` + motiv
  (`NACK_REASON_BAD_CHECKSUM` / `_UNKNOWN_CMD` / `_BAD_LENGTH`) în loc să ignore
  silențios pachetele corupte.
- **Retransmitere la timeout** (master): dacă nu vine răspuns în
  `MASTER_RESPONSE_TIMEOUT_MS` (300 ms), master retrimite același SEQ — până
  la `MASTER_MAX_RETRIES` (3) încercări, după care marchează `FAIL` și trece
  mai departe.
- **Filtru pe ID**: slave-ul acceptă doar `DEVICE_ID_SLAVE` sau broadcast,
  ignoră restul.
- **Dedup după (ID, SEQ)**: fereastră glisantă de 4 cadre — dacă master
  retransmite, slave-ul îl recunoaște și răspunde din nou FĂRĂ să incrementeze
  contorul intern (evită dubla contorizare).
- **Statistici** periodice atât la master (`tx/ok/retries/fail/nack`) cât și la
  slave (`rx/acc/bad_cs/bad_id/dup/cnt`).

## 6. Construire și flash

Proiectul are trei environment-uri PlatformIO:

```bash
pio run                                 # build toate 3 env-urile
pio run -e esp32_master -t upload       # flash pe placa #1 (master)
pio run -e esp32_slave  -t upload       # flash pe placa #2 (slave)
pio device monitor -e esp32_master      # terminal Serial0 pe master
pio device monitor -e esp32_slave       # terminal Serial0 pe slave
```

Cablajul fizic (`diagram.json`):

```
   Master GPIO 17 (TX2) ─── Slave GPIO 16 (RX2)
   Master GPIO 16 (RX2) ─── Slave GPIO 17 (TX2)
   Master GND         ─── Slave GND
```

### 6.1 Simulare în Wokwi (build `esp32_demo`)

Wokwi rulează un singur firmware per simulare, așa că nu putem instanția
două ESP32-uri cu main-uri distincte. Pentru a expune totuși tot protocolul
într-o singură sesiune serial, `esp32_demo` folosește un **UART virtual
in-process**: două cozi FreeRTOS împachetate peste interfața `dd_uart` joacă
rolul firelor TX↔RX. Master-ul și slave-ul rulează pe același chip, dar **tot
codul** (encode, state machine la recepție, FIFO, dedup, ACK/NACK, retry,
statistici) este identic cu build-urile fizice. Singurul element înlocuit este
transportul pe fir.

## 7. Mapare la cerințele lucrării

| Cerință                                                                 | Implementare                                                                             |
|-------------------------------------------------------------------------|------------------------------------------------------------------------------------------|
| 50% — Funcționare completă protocol binar (TX/RX corectă)               | `ctrl_bin_packet` + `dd_uart` (encode/decode + state machine), `app_uart_master/slave`   |
| 10% — FIFO pentru stocarea pachetelor primite                           | `xQueueCreate(SLAVE_RX_FIFO_DEPTH, sizeof(fifo_slot_t))` în `app_uart_slave`             |
| 10% — Task-uri FreeRTOS (recepție, prelucrare, răspuns)                 | `uart_rx` + `pkt_proc` + `uart_tx` (+ `*_diag`)                                          |
| 10% — Structura pachetului, arhitectură sistem, interfețe HW/SW         | Secțiunile 1–4 ale acestui README + diagrama Wokwi                                       |
| 10% — Conexiuni electrice, scheme echipamente                           | `diagram.json` (Wokwi) + tabelul din secțiunea 6                                         |
| 10% — Comportament adițional improvizat                                 | ACK/NACK + retransmisie + filtru ID + dedup + statistici (secțiunea 5)                   |

# Lab 7.2 — Cum se citește log-ul din Wokwi

Mai jos iau log-ul pe care l-ai văzut în Serial Monitor și îl explic
rând pe rând: ce înseamnă fiecare linie, de ce uneori textul pare
"amestecat", și de ce prima cerere se retransmite.

---

## 1. Bootloader ROM (nu ține de firmware)

```
ets Jul 29 2019 12:21:46
rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:1156
load:0x40078000,len:11456
ho 0 tail 12 room 4
load:0x40080400,len:2972
entry 0x400805dc
```

Astea sunt mesajele **ROM-bootloader-ului** de pe ESP32:

| Linie | Ce înseamnă |
|-------|-------------|
| `ets Jul 29 2019 12:21:46` | data build-ului ROM-ului (este aceeași pe toate ESP32-urile) |
| `rst:0x1 (POWERON_RESET)` | motivul resetării: alimentarea cu tensiune |
| `boot:0x13 (SPI_FAST_FLASH_BOOT)` | boot dintr-un flash SPI extern (normal) |
| `configsip`, `clk_drv`, `q_drv`, … | parametrii liniilor SPI către flash |
| `mode:DIO, clock div:2` | SPI în mod DIO (două linii de date) |
| `load:0x3fff0030,len:1156` | se încarcă 1156 B la adresa RAM `0x3fff0030` (secțiunea DRAM) |
| `load:0x40078000,len:11456` | se încarcă a doua etapă bootloader în IRAM |
| `load:0x40080400,len:2972` | firmware-ul (vectorul de reset al Arduino/ESP-IDF) |
| `entry 0x400805dc` | sare la `0x400805dc` = începe execuția codului nostru |

**Regula simplă:** tot ce apare înainte de prima linie de `=====` este de
la Espressif, nu de la noi.

---

## 2. Banner-ul nostru de pornire

```
================================================
   LAB 7.2 — DEMO (Wokwi, UART virtual in-process)
   Master + Slave pe același chip,
   transport prin FIFO (două cozi FreeRTOS)
================================================
```

Este primul lucru pe care îl scrie codul nostru (din `main_demo.cpp`,
funcția `print_boot_banner`). Ne confirmă că:

- firmware-ul s-a pornit cu succes,
- suntem în build-ul `esp32_demo` (nu `esp32_master` sau `esp32_slave`),
- master și slave vor rula pe același chip, conectați prin două cozi
  FreeRTOS care joacă rolul firului TX↔RX.

---

## 3. Mesajele de inițializare (Slave / Master / DEMO)

```
[SLAVE] gata (DEMO / UART virtual). ID=0x02, FIFO=8

[MASTER][ TX ] PING  SEQ=0  len=0
[MASTER] gata (DEMO / UART virtual). ID=0x01 -> slave 0x02
[DEMO] Legătura virtuală master<->slave e activă.
```

- `[SLAVE] gata ...` — slave-ul a creat FIFO-ul (8 sloturi de pachete),
  mutex-ul, și cele 3 task-uri (`uart_rx`, `pkt_proc`, `slave_diag`).
  ID-ul lui logic este `0x02`.
- `[MASTER][ TX ] PING  SEQ=0  len=0` — aici vezi o particularitate: master-ul
  a apucat să **trimită deja primul pachet** înainte ca linia lui proprie
  de "gata" să apuce să fie tipărită. De ce? Pentru că `app_uart_master_init`
  creează task-urile ÎN TIMP CE încă mai printează banner-ul de "gata" —
  task-ul `uart_tx` pornește, ia imediat `CMD_PING`, îl encodează și îl pune
  pe UART-ul virtual; abia apoi Arduino-task-ul apucă să termine de scris
  `[MASTER] gata ...`. Ordinea care apare în terminal ≠ ordinea "logică" —
  RTOS nu garantează că un task pornit recent nu va ajunge la o operație
  rapidă înainte să se termine o altă operație mai veche.
- `[DEMO] Legătura virtuală master<->slave e activă.` — înseamnă că
  `setup()` a terminat: ambele capete (`app_uart_slave` și
  `app_uart_master`) au fost create și apoi **legate** de cele două cozi
  (m→s și s→m) prin `dd_uart_bind_virtual_transport(...)`.

---

## 4. Prima retransmisie (`SEQ=0`)

```
[MASTER][RTRY] SEQ=0, încercarea 1/3
 MASTER][[S LTAXV E]] [P IRNXG  ]  SPEIQN=G0  S EleQn==00
 => PONG (uptime 4B)
[MASTER][ OK ] PONG  SEQ=0  slave uptime=804 ms
```

De ce prima cerere se retransmite? **Curse de pornire**.

- La `t=0`, master-ul deja a pus `PING  SEQ=0` în coada m→s.
- În acel moment slave-ul are task-urile create, dar încă nu a fost legat
  pe transportul virtual (în `main_demo.cpp` legarea slave-ului vine
  înainte de `app_uart_master_init`, iar apoi legarea master-ului). Timing-
  ul exact depinde de când schedulerul dă time-slice fiecărui task.
- Master-ul așteaptă `MASTER_RESPONSE_TIMEOUT_MS = 300 ms`. Dacă între
  timp slave-ul nu a consumat cadrul, master-ul marchează **timeout** și
  face retransmisie (`RTRY 1/3`).
- Imediat după retransmisie, slave-ul citește cadrul, îl validează, iar
  mecanismul lui de **dedup pe `(ID, SEQ)`** îl recunoaște ca "deja văzut"
  (sau execută pentru prima dată, dacă a ratat originalul); apoi trimite
  `PONG` înapoi.
- Master-ul primește `PONG SEQ=0`, se potrivește cu SEQ-ul cererii, marchează
  `OK` și treaba se stabilizează. Toate cererile următoare vin instant.

**Concluzie:** o retransmisie la startup este un efect secundar al
concurenței, nu un bug. Exact pentru asta am pus mecanismul de retry —
să tolereze exact astfel de situații, sau pe hardware un glitch de cablu.

---

## 5. Textul "amestecat" pe un rând

```
 MASTER][[S LTAXV E]] [P IRNXG  ]  SPEIQN=G0  S EleQn==00
```

Acesta NU este o eroare — este un **efect de concurență la afișare**.

- `ctrl_stdio_printf(...)` apelează în final `Serial.write(buffer, n)`.
  Pe ESP32 Arduino `Serial.write` este **thread-safe la nivel de octet**
  (îl protejează un mutex intern), dar NU la nivel de "întreg string".
- Dacă **task-ul TX al master-ului** și **task-ul procesare al slave-ului**
  ambele vor să scrie un string în exact aceeași milisecundă, octeții lor
  pot fi intercalați.
- Deci rândul de mai sus e de fapt două rânduri suprapuse caracter cu caracter:
  - `[MASTER][ TX ] PING  SEQ=0  len=0`
  - `[SLAVE][ RX ] PING SEQ=0  => PONG (uptime 4B)`
  Dacă le citești pe coloane, le recunoști.
- Același pattern vezi mai jos, cu `[SLAVE][stats]` intercalat în
  `[MASTER][stats]`. La nivel de **protocol** nu s-a pierdut nimic — e doar
  afișarea.

> **Dacă vrei rânduri curate**, o soluție simplă e să punem un mutex în jurul
> întregului apel `ctrl_stdio_printf(...)` — toate task-urile așteaptă
> unul după altul înainte să scrie. Nu am făcut-o pentru că pe hardware-ul
> real (două plăci separate) fiecare plată are propriul Serial Monitor și
> problema dispare — e specifică demo-ului single-chip.

---

## 6. Fluxul standard după stabilizare

De la `SEQ=1` în sus, fiecare cerere arată frumos:

```
[MASTER][ TX ] GET_COUNTER  SEQ=1  len=0
[SLAVE][ RX ] GET_COUNTER SEQ=1  => cnt=2
[MASTER][ OK ] COUNTER_VALUE SEQ=1  cnt=2
```

Traducere:
1. Master encodează `[STX=0x02] [ID=0x02] [SEQ=0x01] [CMD=0x20]
   [LEN=0x00] [CS=0x23] [ETX=0x03]` și îl pune pe "fir" (coada
   master→slave).
2. Task-ul RX al slave-ului extrage cadrul prin state-machine-ul de parser
   (verifică STX, LEN, ETX, recalculează XOR-checksum), îl pune în FIFO.
3. `task_packet_processor` scoate cadrul din FIFO, decide "e pentru mine"
   (ID = 0x02, `DEVICE_ID_SLAVE`), incrementează contorul intern și trimite
   înapoi `CMD_COUNTER_VALUE` cu valoarea pe 4 bytes (big-endian) în payload.
4. Master-ul primește răspunsul, verifică SEQ-ul (să corespundă cererii în
   zbor), decodifică cei 4 bytes BE într-un `uint32_t` și tipărește `cnt=2`.

Master-ul **alternează** `PING` și `GET_COUNTER` (vezi `tick % 2` din
`task_master_tx`), ca să vezi ambele fluxuri — inclusiv payload diferit
(timestamp vs. contor).

---

## 7. Mesajele periodice `[stats]`

```
[SLAVE][stats] rx=5 acc=5 bad_cs=0 bad_id=0 dup=0 cnt=5
[MASTER][stats] tx=6 ok=5 retries=1 fail=0 nack=0
```

Contoarele de pe slave (`task_diag`):

| Contor | Ce înseamnă |
|--------|-------------|
| `rx`      | câte cadre brute a extras state-machine-ul cu STX+ETX valide |
| `acc`     | câte au trecut toate filtrele (ID corect, checksum OK, dedup) |
| `bad_cs`  | câte au pica pe checksum greșit → răspuns `NACK(BAD_CHECKSUM)` |
| `bad_id`  | câte erau adresate altcuiva (filtru pe ID, ignorate silențios) |
| `dup`     | câte erau retransmisii recunoscute → răspundem dar nu contorizăm |
| `cnt`     | cât afișăm în `CMD_COUNTER_VALUE` |

Contoarele de pe master (`task_diag`):

| Contor | Ce înseamnă |
|--------|-------------|
| `tx`      | cereri emise (inclusiv retransmisiile) |
| `ok`      | răspunsuri `PONG` / `COUNTER_VALUE` primite și potrivite cu SEQ-ul |
| `retries` | câte retransmisii am făcut (e 1 aici = `SEQ=0` de la startup) |
| `fail`    | câte cereri au eșuat definitiv (3 retry-uri fără răspuns) |
| `nack`    | câte `NACK` am primit |

În log-ul tău: `tx=6, ok=5, retries=1, fail=0` = totul OK, singura
retransmisie fiind cea de startup, explicată mai sus.

---

## 8. Când să-ți faci griji (și când nu)

| Văd în log | Normal? | Acțiune |
|------------|---------|---------|
| `MASTER][RTRY] SEQ=0 ... 1/3` la pornire | **Da** | Ignoră, e race-ul de inițializare |
| `MASTER][RTRY]` la orice `SEQ > 0` | nu | Verifică cablajul TX↔RX / GND (pe hardware fizic) |
| `MASTER][FAIL]` pe o cerere | nu | Slave-ul nu răspunde deloc — problemă mai serioasă |
| `SLAVE][NACK] ... checksum invalid` | nu | Probleme pe linie (paraziți, baudrate greșit) |
| `SLAVE][IGN] Pachet pentru ID=...` | **Da** | Cineva trimite pachete adresate altcuiva, le ignorăm |
| `SLAVE][DUP ]` | **Da** | Master a retransmis, slave a recunoscut |
| Text intercalat char-cu-char | **Da** (doar în DEMO) | Efect de concurență, nu afectează protocolul |

---

## 9. Ce confirmă acest log

1. **Protocolul binar funcționează end-to-end**: encode → UART → state-machine
   → FIFO → decode → răspuns → match pe SEQ. Cerința 50%.
2. **FIFO-ul există și e folosit**: fiecare `[SLAVE][ RX ]` este un element
   scos din FIFO. Cerința 10%.
3. **Task-urile FreeRTOS sunt distincte**: vezi clar `MASTER`, `SLAVE`,
   `[stats]` — sunt task-uri separate care rulează concurent (de aici și
   textul amestecat). Cerința 10%.
4. **Comportamentul adițional lucrează**: vezi `RTRY 1/3` și `OK` imediat
   după — mecanismul de retransmisie la timeout funcționează. Cerința 10%
   pentru "comportament adițional improvizat".

#ifndef CONFIGS_H
#define CONFIGS_H

#include <Arduino.h>
#include <stdint.h>

// ============================================================================
//  LAB 7.2 — Configurații centralizate (no magic numbers)
// ============================================================================
//  Tot proiectul împarte constantele de hardware, protocol și task-uri prin
//  acest header. Orice modificare (pini, baudrate, comenzi) se face strict
//  într-un singur loc.
// ============================================================================

// ---------- STDIO (debug serial — nu atinge traficul binar) -----------------
#define SERIAL_BAUDRATE                 115200UL

// ---------- UART binar dedicat (Serial2 pe ambele plăci) --------------------
// Convenție: fiecare placă TRANSMITE pe pinul său TX și PRIMEȘTE pe pinul RX.
// Cablajul fizic încrucișat (TX_A -> RX_B, TX_B -> RX_A) e ilustrat în README.
#define BIN_UART_BAUDRATE               9600UL
#define BIN_UART_RX_PIN                 16
#define BIN_UART_TX_PIN                 17
#define BIN_UART_PORT_NUMBER            2        // HardwareSerial(2) -> Serial2
#define BIN_UART_RX_BUFFER_SIZE         256

// Pinii pentru build-ul DEMO single-chip (două magistrale virtuale, cablate
// cap la cap prin FIFO in-process — Wokwi nu poate cros-ABL UART-urile fizic).
#define BIN_UART_DEMO_MASTER_RX_PIN     25
#define BIN_UART_DEMO_MASTER_TX_PIN     26
#define BIN_UART_DEMO_SLAVE_RX_PIN      27
#define BIN_UART_DEMO_SLAVE_TX_PIN      14

// ---------- Identificatori logici ai dispozitivelor -------------------------
#define DEVICE_ID_MASTER                0x01
#define DEVICE_ID_SLAVE                 0x02
#define DEVICE_ID_BROADCAST             0xFF

// ---------- Delimitatori binari pentru protocolul logic ---------------------
#define PROTO_START_BYTE                0x02     // STX
#define PROTO_END_BYTE                  0x03     // ETX

// ---------- Capacități ale protocolului -------------------------------------
#define PROTO_MAX_PAYLOAD_SIZE          16
// Structură: START(1) + ID(1) + SEQ(1) + CMD(1) + LEN(1) + PAYLOAD + CS(1) + END(1)
#define PROTO_FRAME_OVERHEAD_BYTES      7
#define PROTO_MAX_FRAME_SIZE            (PROTO_FRAME_OVERHEAD_BYTES + PROTO_MAX_PAYLOAD_SIZE)

// ---------- Codurile comenzilor ---------------------------------------------
#define CMD_PING                        0x10   // master -> slave
#define CMD_PONG                        0x11   // slave -> master (răspuns la PING)
#define CMD_GET_COUNTER                 0x20   // master -> slave
#define CMD_COUNTER_VALUE               0x21   // slave -> master (4 bytes BE în payload)
#define CMD_ACK                         0x30   // confirmare generică
#define CMD_NACK                        0x31   // pachet invalid (checksum / structură)

// Coduri de eroare folosite în payload-ul NACK
#define NACK_REASON_BAD_CHECKSUM        0x01
#define NACK_REASON_UNKNOWN_CMD         0x02
#define NACK_REASON_BAD_LENGTH          0x03

// ---------- FIFO de pachete valide pe slave ---------------------------------
#define SLAVE_RX_FIFO_DEPTH             8

// ---------- Duplicate filter (bonus) ----------------------------------------
// Slave-ul memorează ultimele N SEQ primite per ID — util ca să nu execute de
// două ori aceeași cerere atunci când master-ul retransmite.
#define SLAVE_DEDUP_WINDOW              4

// ---------- Task timing & priority ------------------------------------------
#define TASK_STACK_SIZE                 4096

#define TASK_PRIORITY_UART_RX           3    // cel mai prioritar — fără să piardă bytes
#define TASK_PRIORITY_PACKET_PROC       2
#define TASK_PRIORITY_MASTER_TX         2
#define TASK_PRIORITY_DIAG              1

#define MASTER_REQUEST_PERIOD_MS        1000   // cât de des interoghează master-ul
#define MASTER_RESPONSE_TIMEOUT_MS      300    // cât așteaptă un răspuns înainte să retransmită
#define MASTER_MAX_RETRIES              3      // câte retransmisii înainte să marcheze fail
#define MASTER_RETRY_GAP_MS             50     // pauza minimă între două retransmisii

#define UART_RX_TASK_IDLE_TICK_MS       2      // poll UART rapid
#define SLAVE_DIAG_PERIOD_MS            5000   // cât de des tipărește slave-ul statisticile

#endif // CONFIGS_H

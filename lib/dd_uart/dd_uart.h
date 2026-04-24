#ifndef DD_UART_H
#define DD_UART_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <stddef.h>
#include <stdint.h>

#include "configs.h"

// ============================================================================
//  dd_uart — HAL peste HardwareSerial (Serial1/Serial2) pentru traficul binar.
// ============================================================================
//  Oferă:
//   - inițializare a portului (baudrate, pinii RX/TX)
//   - scriere raw de octeți
//   - parser incremental cu state machine pentru extragerea cadrelor
//     [STX][ID][SEQ][CMD][LEN][PAYLOAD][CS][ETX] dintr-un stream.
//
//  Pentru build-ul DEMO se poate crea o instanță fără HardwareSerial (bus =
//  nullptr). În acest caz `dd_uart_write_bytes` și `dd_uart_read_byte` sunt
//  delegate către callback-uri externe — vezi `dd_uart_bind_virtual_transport`.

typedef int (*dd_uart_read_byte_fn)(void* context);
typedef size_t (*dd_uart_write_bytes_fn)(void* context,
                                         const uint8_t* data,
                                         size_t length);

typedef enum {
    DD_UART_STATE_WAIT_START = 0,
    DD_UART_STATE_ID,
    DD_UART_STATE_SEQ,
    DD_UART_STATE_CMD,
    DD_UART_STATE_LEN,
    DD_UART_STATE_PAYLOAD,
    DD_UART_STATE_CHECKSUM,
    DD_UART_STATE_END,
} dd_uart_parser_state_t;

typedef struct {
    HardwareSerial*        hw;             // poate fi nullptr în build DEMO
    uint8_t                frame[PROTO_MAX_FRAME_SIZE];
    size_t                 frame_index;
    uint8_t                payload_remaining;
    dd_uart_parser_state_t state;

    // Transport virtual (doar în DEMO)
    dd_uart_read_byte_fn   v_read;
    dd_uart_write_bytes_fn v_write;
    void*                  v_context;
} dd_uart_t;

// Inițializare pe hardware real.
void dd_uart_init(dd_uart_t* u,
                  HardwareSerial* hw,
                  unsigned long baudrate,
                  int8_t rx_pin,
                  int8_t tx_pin);

// Inițializare pentru transport virtual (DEMO): nu atinge hardware-ul.
void dd_uart_bind_virtual_transport(dd_uart_t* u,
                                    dd_uart_read_byte_fn  reader,
                                    dd_uart_write_bytes_fn writer,
                                    void* context);

// Trimite raw octeți. Returnează câți au fost scriși.
size_t dd_uart_write_bytes(dd_uart_t* u, const uint8_t* data, size_t length);

// Hrănește parser-ul cu 1 octet. Când se adună un cadru complet valid,
// returnează `true`, iar `*out_frame_length` spune câți octeți s-au
// acumulat în `u->frame`. Pe eroare (start byte greșit, length ieșit din
// spec) parser-ul se resetează și returnează `false`.
bool dd_uart_feed_byte(dd_uart_t* u, uint8_t byte, size_t* out_frame_length);

// Golește complet starea parser-ului.
void dd_uart_reset_parser(dd_uart_t* u);

// Citește 0 sau 1 octet (neblocant). Returnează -1 dacă nu e nimic disponibil.
int dd_uart_read_byte(dd_uart_t* u);

#endif // DD_UART_H

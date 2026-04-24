#ifndef APP_UART_MASTER_H
#define APP_UART_MASTER_H

#include <HardwareSerial.h>
#include <stddef.h>
#include <stdint.h>

#include "dd_uart.h"

// Inițializează master-ul pe transport-ul indicat și pornește două task-uri:
// un task TX care trimite periodic cereri cu SEQ crescător, și un task RX
// care decodează răspunsurile (ACK / PONG / COUNTER_VALUE / NACK) și le
// afișează prin STDIO. Suportă retransmisie până la MASTER_MAX_RETRIES la
// expirarea MASTER_RESPONSE_TIMEOUT_MS.
void app_uart_master_init(HardwareSerial* hw,
                          unsigned long baudrate,
                          int8_t rx_pin,
                          int8_t tx_pin,
                          uint8_t master_id,
                          uint8_t slave_id);

// Expune `dd_uart_t` intern pentru a-l putea lega de un transport virtual
// (DEMO).
dd_uart_t* app_uart_master_get_uart();

#endif // APP_UART_MASTER_H

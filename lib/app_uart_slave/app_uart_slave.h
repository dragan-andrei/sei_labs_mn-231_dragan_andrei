#ifndef APP_UART_SLAVE_H
#define APP_UART_SLAVE_H

#include <HardwareSerial.h>
#include <stddef.h>
#include <stdint.h>

#include "dd_uart.h"

// Inițializează slave-ul pe transport-ul indicat. Pornește două task-uri
// FreeRTOS: un RX task care golește UART-ul și împinge cadrele valide într-un
// FIFO, și un task de procesare care consumă din FIFO, validează și răspunde.
//
// `hw == nullptr` este acceptat pentru build-ul DEMO (se vor folosi
// `app_uart_slave_virtual_rx_push` / transportul virtual legat înainte de
// apel).
void app_uart_slave_init(HardwareSerial* hw,
                         unsigned long baudrate,
                         int8_t rx_pin,
                         int8_t tx_pin,
                         uint8_t device_id);

// Expune `dd_uart_t` intern pentru a-l putea lega de un transport virtual.
dd_uart_t* app_uart_slave_get_uart();

#endif // APP_UART_SLAVE_H

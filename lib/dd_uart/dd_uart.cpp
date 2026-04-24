#include "dd_uart.h"

#include "ctrl_stdio.h"

void dd_uart_init(dd_uart_t* u,
                  HardwareSerial* hw,
                  unsigned long baudrate,
                  int8_t rx_pin,
                  int8_t tx_pin) {
    if (u == nullptr) {
        return;
    }
    u->hw        = hw;
    u->v_read    = nullptr;
    u->v_write   = nullptr;
    u->v_context = nullptr;
    dd_uart_reset_parser(u);

    if (hw != nullptr) {
        hw->setRxBufferSize(BIN_UART_RX_BUFFER_SIZE);
        hw->begin(baudrate, SERIAL_8N1, rx_pin, tx_pin);
    }
}

void dd_uart_bind_virtual_transport(dd_uart_t* u,
                                    dd_uart_read_byte_fn  reader,
                                    dd_uart_write_bytes_fn writer,
                                    void* context) {
    if (u == nullptr) {
        return;
    }
    u->hw        = nullptr;
    u->v_read    = reader;
    u->v_write   = writer;
    u->v_context = context;
    dd_uart_reset_parser(u);
}

size_t dd_uart_write_bytes(dd_uart_t* u, const uint8_t* data, size_t length) {
    if (u == nullptr || data == nullptr || length == 0) {
        return 0;
    }
    if (u->hw != nullptr) {
        return u->hw->write(data, length);
    }
    if (u->v_write != nullptr) {
        return u->v_write(u->v_context, data, length);
    }
    return 0;
}

int dd_uart_read_byte(dd_uart_t* u) {
    if (u == nullptr) {
        return -1;
    }
    if (u->hw != nullptr) {
        if (u->hw->available() <= 0) {
            return -1;
        }
        return u->hw->read();
    }
    if (u->v_read != nullptr) {
        return u->v_read(u->v_context);
    }
    return -1;
}

void dd_uart_reset_parser(dd_uart_t* u) {
    if (u == nullptr) {
        return;
    }
    u->frame_index       = 0;
    u->payload_remaining = 0;
    u->state             = DD_UART_STATE_WAIT_START;
}

bool dd_uart_feed_byte(dd_uart_t* u, uint8_t byte, size_t* out_frame_length) {
    if (u == nullptr) {
        return false;
    }

    switch (u->state) {
        case DD_UART_STATE_WAIT_START: {
            if (byte != PROTO_START_BYTE) {
                // Resincronizare: ignorăm silențios octeții zgomot dinainte
                // de următorul STX.
                return false;
            }
            u->frame_index = 0;
            u->frame[u->frame_index++] = byte;
            u->state = DD_UART_STATE_ID;
            return false;
        }
        case DD_UART_STATE_ID: {
            u->frame[u->frame_index++] = byte;
            u->state = DD_UART_STATE_SEQ;
            return false;
        }
        case DD_UART_STATE_SEQ: {
            u->frame[u->frame_index++] = byte;
            u->state = DD_UART_STATE_CMD;
            return false;
        }
        case DD_UART_STATE_CMD: {
            u->frame[u->frame_index++] = byte;
            u->state = DD_UART_STATE_LEN;
            return false;
        }
        case DD_UART_STATE_LEN: {
            u->frame[u->frame_index++] = byte;
            if (byte > PROTO_MAX_PAYLOAD_SIZE) {
                ctrl_stdio_printf("[UART][ERR] LEN=%u > MAX, resincronizez\n",
                                  static_cast<unsigned>(byte));
                dd_uart_reset_parser(u);
                return false;
            }
            u->payload_remaining = byte;
            u->state = (u->payload_remaining == 0) ? DD_UART_STATE_CHECKSUM
                                                   : DD_UART_STATE_PAYLOAD;
            return false;
        }
        case DD_UART_STATE_PAYLOAD: {
            u->frame[u->frame_index++] = byte;
            if (--u->payload_remaining == 0) {
                u->state = DD_UART_STATE_CHECKSUM;
            }
            return false;
        }
        case DD_UART_STATE_CHECKSUM: {
            u->frame[u->frame_index++] = byte;
            u->state = DD_UART_STATE_END;
            return false;
        }
        case DD_UART_STATE_END: {
            u->frame[u->frame_index++] = byte;
            if (byte != PROTO_END_BYTE) {
                ctrl_stdio_printf(
                    "[UART][ERR] ETX lipsă (0x%02X), resincronizez\n",
                    static_cast<unsigned>(byte));
                dd_uart_reset_parser(u);
                return false;
            }
            if (out_frame_length != nullptr) {
                *out_frame_length = u->frame_index;
            }
            const size_t len = u->frame_index;
            dd_uart_reset_parser(u);
            (void)len;
            return true;
        }
    }
    dd_uart_reset_parser(u);
    return false;
}

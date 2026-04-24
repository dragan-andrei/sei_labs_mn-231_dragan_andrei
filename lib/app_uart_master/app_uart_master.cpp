#include "app_uart_master.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "configs.h"
#include "ctrl_bin_packet.h"
#include "ctrl_stdio.h"

namespace {

dd_uart_t     g_uart;
uint8_t       g_master_id  = DEVICE_ID_MASTER;
uint8_t       g_slave_id   = DEVICE_ID_SLAVE;
uint8_t       g_next_seq   = 0;

// Starea "cererii în zbor" pentru mecanismul de ACK / retransmitere.
SemaphoreHandle_t g_inflight_mutex = nullptr;
volatile bool     g_inflight_active    = false;
volatile uint8_t  g_inflight_seq       = 0;
volatile uint8_t  g_inflight_expected_cmd = 0;
volatile uint8_t  g_inflight_retries   = 0;
volatile uint32_t g_inflight_deadline  = 0;

// Statistici
volatile uint32_t g_stat_sent        = 0;
volatile uint32_t g_stat_ok          = 0;
volatile uint32_t g_stat_retries     = 0;
volatile uint32_t g_stat_failed      = 0;
volatile uint32_t g_stat_nacks       = 0;

void send_packet(const bin_packet_t& pkt) {
    uint8_t buffer[PROTO_MAX_FRAME_SIZE];
    const size_t n = ctrl_bin_packet_encode(&pkt, buffer, sizeof(buffer));
    if (n == 0) {
        ctrl_stdio_print_text("[MASTER][ERR] Encode a eșuat\n");
        return;
    }
    dd_uart_write_bytes(&g_uart, buffer, n);
    ctrl_stdio_printf(
        "[MASTER][ TX ] %s  SEQ=%u  len=%u\n",
        ctrl_bin_packet_cmd_name(pkt.cmd),
        static_cast<unsigned>(pkt.seq),
        static_cast<unsigned>(pkt.length));
    ++g_stat_sent;
}

void build_request(uint8_t cmd, uint8_t seq, bin_packet_t* out) {
    out->id     = g_slave_id;       // destinatar
    out->seq    = seq;
    out->cmd    = cmd;
    out->length = 0;
}

uint8_t expected_response_cmd(uint8_t request_cmd) {
    switch (request_cmd) {
        case CMD_PING:        return CMD_PONG;
        case CMD_GET_COUNTER: return CMD_COUNTER_VALUE;
        default:              return CMD_ACK;
    }
}

void start_request(uint8_t cmd) {
    if (xSemaphoreTake(g_inflight_mutex, portMAX_DELAY) != pdTRUE) {
        return;
    }
    const uint8_t seq = g_next_seq++;
    g_inflight_active       = true;
    g_inflight_seq          = seq;
    g_inflight_expected_cmd = expected_response_cmd(cmd);
    g_inflight_retries      = 0;
    g_inflight_deadline     = millis() + MASTER_RESPONSE_TIMEOUT_MS;

    bin_packet_t req;
    build_request(cmd, seq, &req);
    send_packet(req);
    xSemaphoreGive(g_inflight_mutex);
}

void resend_current(uint8_t cmd) {
    // apelat cu mutex-ul deja luat
    g_inflight_retries++;
    g_inflight_deadline = millis() + MASTER_RESPONSE_TIMEOUT_MS;
    ++g_stat_retries;
    ctrl_stdio_printf(
        "[MASTER][RTRY] SEQ=%u, încercarea %u/%u\n",
        static_cast<unsigned>(g_inflight_seq),
        static_cast<unsigned>(g_inflight_retries),
        static_cast<unsigned>(MASTER_MAX_RETRIES));
    bin_packet_t req;
    build_request(cmd, g_inflight_seq, &req);
    send_packet(req);
}

void clear_inflight() {
    // apelat cu mutex-ul deja luat
    g_inflight_active = false;
}

void print_response(const bin_packet_t& resp) {
    switch (resp.cmd) {
        case CMD_PONG: {
            uint32_t uptime_ms = 0;
            if (resp.length >= 4) {
                uptime_ms = (static_cast<uint32_t>(resp.payload[0]) << 24) |
                            (static_cast<uint32_t>(resp.payload[1]) << 16) |
                            (static_cast<uint32_t>(resp.payload[2]) << 8)  |
                            (static_cast<uint32_t>(resp.payload[3]));
            }
            ctrl_stdio_printf(
                "[MASTER][ OK ] PONG  SEQ=%u  slave uptime=%lu ms\n",
                static_cast<unsigned>(resp.seq),
                static_cast<unsigned long>(uptime_ms));
            break;
        }
        case CMD_COUNTER_VALUE: {
            uint32_t c = 0;
            if (resp.length >= 4) {
                c = (static_cast<uint32_t>(resp.payload[0]) << 24) |
                    (static_cast<uint32_t>(resp.payload[1]) << 16) |
                    (static_cast<uint32_t>(resp.payload[2]) << 8)  |
                    (static_cast<uint32_t>(resp.payload[3]));
            }
            ctrl_stdio_printf(
                "[MASTER][ OK ] COUNTER_VALUE SEQ=%u  cnt=%lu\n",
                static_cast<unsigned>(resp.seq),
                static_cast<unsigned long>(c));
            break;
        }
        case CMD_NACK: {
            const uint8_t reason = (resp.length >= 2) ? resp.payload[1] : 0;
            ctrl_stdio_printf(
                "[MASTER][NACK] SEQ=%u  motiv=0x%02X\n",
                static_cast<unsigned>(resp.seq),
                static_cast<unsigned>(reason));
            ++g_stat_nacks;
            break;
        }
        default:
            ctrl_stdio_printf(
                "[MASTER][ ?? ] CMD=0x%02X SEQ=%u len=%u\n",
                static_cast<unsigned>(resp.cmd),
                static_cast<unsigned>(resp.seq),
                static_cast<unsigned>(resp.length));
            break;
    }
}

void handle_response(const bin_packet_t& resp) {
    xSemaphoreTake(g_inflight_mutex, portMAX_DELAY);
    print_response(resp);

    if (!g_inflight_active) {
        ctrl_stdio_print_text(
            "[MASTER][WARN] Răspuns în afara unei cereri în zbor\n");
        xSemaphoreGive(g_inflight_mutex);
        return;
    }
    if (resp.seq != g_inflight_seq) {
        ctrl_stdio_printf(
            "[MASTER][WARN] SEQ-ul răspunsului (%u) nu se potrivește (aștept %u)\n",
            static_cast<unsigned>(resp.seq),
            static_cast<unsigned>(g_inflight_seq));
        xSemaphoreGive(g_inflight_mutex);
        return;
    }

    if (resp.cmd == g_inflight_expected_cmd || resp.cmd == CMD_ACK) {
        ++g_stat_ok;
    } else if (resp.cmd == CMD_NACK) {
        ++g_stat_failed;
    }
    clear_inflight();
    xSemaphoreGive(g_inflight_mutex);
}

void task_master_tx(void* parameters) {
    (void)parameters;
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(MASTER_REQUEST_PERIOD_MS);
    uint32_t tick = 0;

    for (;;) {
        // Decidem ce comandă trimitem: alternăm PING și GET_COUNTER ca să
        // vedem ambele fluxuri, inclusiv payload-ul.
        const uint8_t cmd = (tick++ % 2 == 0) ? CMD_PING : CMD_GET_COUNTER;

        ctrl_stdio_print_text("\n");
        start_request(cmd);

        // Bucla de așteptare a răspunsului cu retransmisie.
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(MASTER_RETRY_GAP_MS));

            bool done = false;
            bool timeout = false;
            xSemaphoreTake(g_inflight_mutex, portMAX_DELAY);
            if (!g_inflight_active) {
                done = true;
            } else if (static_cast<int32_t>(millis() - g_inflight_deadline) >= 0) {
                timeout = true;
            }

            if (timeout && g_inflight_retries < MASTER_MAX_RETRIES) {
                resend_current(cmd);
                xSemaphoreGive(g_inflight_mutex);
                continue;
            }
            if (timeout) {
                ctrl_stdio_printf(
                    "[MASTER][FAIL] SEQ=%u — nu a răspuns după %u încercări\n",
                    static_cast<unsigned>(g_inflight_seq),
                    static_cast<unsigned>(MASTER_MAX_RETRIES));
                ++g_stat_failed;
                clear_inflight();
                done = true;
            }
            xSemaphoreGive(g_inflight_mutex);
            if (done) {
                break;
            }
        }

        vTaskDelayUntil(&last_wake, period);
    }
}

void task_master_rx(void* parameters) {
    (void)parameters;
    for (;;) {
        int b = dd_uart_read_byte(&g_uart);
        while (b >= 0) {
            size_t frame_len = 0;
            if (dd_uart_feed_byte(&g_uart,
                                  static_cast<uint8_t>(b),
                                  &frame_len)) {
                bin_packet_t resp;
                const bin_decode_result_t r =
                    ctrl_bin_packet_decode(g_uart.frame, frame_len, &resp);
                if (r == BIN_DECODE_OK) {
                    handle_response(resp);
                } else {
                    ctrl_stdio_printf(
                        "[MASTER][ERR] Cadru corupt (r=%u, %u octeți)\n",
                        static_cast<unsigned>(r),
                        static_cast<unsigned>(frame_len));
                }
            }
            b = dd_uart_read_byte(&g_uart);
        }
        vTaskDelay(pdMS_TO_TICKS(UART_RX_TASK_IDLE_TICK_MS));
    }
}

void task_diag(void* parameters) {
    (void)parameters;
    TickType_t last = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(SLAVE_DIAG_PERIOD_MS);
    for (;;) {
        vTaskDelayUntil(&last, period);
        ctrl_stdio_printf(
            "[MASTER][stats] tx=%lu ok=%lu retries=%lu fail=%lu nack=%lu\n",
            static_cast<unsigned long>(g_stat_sent),
            static_cast<unsigned long>(g_stat_ok),
            static_cast<unsigned long>(g_stat_retries),
            static_cast<unsigned long>(g_stat_failed),
            static_cast<unsigned long>(g_stat_nacks));
    }
}

}  // namespace

void app_uart_master_init(HardwareSerial* hw,
                          unsigned long baudrate,
                          int8_t rx_pin,
                          int8_t tx_pin,
                          uint8_t master_id,
                          uint8_t slave_id) {
    g_master_id = master_id;
    g_slave_id  = slave_id;

    dd_uart_init(&g_uart, hw, baudrate, rx_pin, tx_pin);

    g_inflight_mutex = xSemaphoreCreateMutex();
    if (g_inflight_mutex == nullptr) {
        ctrl_stdio_print_text("[MASTER][EROARE] Nu pot aloca mutex-ul\n");
        return;
    }

    const BaseType_t tx_ok = xTaskCreate(
        task_master_tx, "uart_tx", TASK_STACK_SIZE, nullptr,
        TASK_PRIORITY_MASTER_TX, nullptr);
    const BaseType_t rx_ok = xTaskCreate(
        task_master_rx, "uart_rx", TASK_STACK_SIZE, nullptr,
        TASK_PRIORITY_UART_RX, nullptr);
    const BaseType_t dg_ok = xTaskCreate(
        task_diag, "master_diag", TASK_STACK_SIZE, nullptr,
        TASK_PRIORITY_DIAG, nullptr);
    if (tx_ok != pdPASS || rx_ok != pdPASS || dg_ok != pdPASS) {
        ctrl_stdio_print_text("[MASTER][EROARE] Task-uri eșuate\n");
        return;
    }

    if (hw != nullptr) {
        ctrl_stdio_printf(
            "[MASTER] gata. ID=0x%02X -> slave 0x%02X, baud=%lu (RX=%d, TX=%d)\n",
            static_cast<unsigned>(g_master_id),
            static_cast<unsigned>(g_slave_id),
            static_cast<unsigned long>(baudrate),
            static_cast<int>(rx_pin),
            static_cast<int>(tx_pin));
    } else {
        ctrl_stdio_printf(
            "[MASTER] gata (DEMO / UART virtual). ID=0x%02X -> slave 0x%02X\n",
            static_cast<unsigned>(g_master_id),
            static_cast<unsigned>(g_slave_id));
    }
}

dd_uart_t* app_uart_master_get_uart() {
    return &g_uart;
}

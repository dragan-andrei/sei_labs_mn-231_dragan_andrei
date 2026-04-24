#include "app_uart_slave.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "configs.h"
#include "ctrl_bin_packet.h"
#include "ctrl_stdio.h"

namespace {

// Un slot în FIFO păstrează cadrul RAW — nu structura decodată — ca să nu
// pierdem informație utilă de diagnostic la pachetele cu checksum invalid.
struct fifo_slot_t {
    uint8_t  frame[PROTO_MAX_FRAME_SIZE];
    uint16_t length;
};

dd_uart_t     g_uart;
uint8_t       g_device_id = DEVICE_ID_SLAVE;
QueueHandle_t g_rx_fifo   = nullptr;

// Statistici + dedup pentru diagnostic
volatile uint32_t g_stat_received      = 0;
volatile uint32_t g_stat_accepted      = 0;
volatile uint32_t g_stat_bad_checksum  = 0;
volatile uint32_t g_stat_bad_id        = 0;
volatile uint32_t g_stat_duplicates    = 0;
volatile uint32_t g_app_counter        = 0;

struct dedup_entry_t {
    uint8_t id;
    uint8_t seq;
    bool    used;
};
dedup_entry_t g_dedup[SLAVE_DEDUP_WINDOW];
uint8_t       g_dedup_head = 0;

bool dedup_seen(uint8_t id, uint8_t seq) {
    for (uint8_t i = 0; i < SLAVE_DEDUP_WINDOW; ++i) {
        if (g_dedup[i].used && g_dedup[i].id == id && g_dedup[i].seq == seq) {
            return true;
        }
    }
    return false;
}

void dedup_record(uint8_t id, uint8_t seq) {
    g_dedup[g_dedup_head].id   = id;
    g_dedup[g_dedup_head].seq  = seq;
    g_dedup[g_dedup_head].used = true;
    g_dedup_head = static_cast<uint8_t>((g_dedup_head + 1) % SLAVE_DEDUP_WINDOW);
}

void send_response(const bin_packet_t& packet) {
    uint8_t tx_buffer[PROTO_MAX_FRAME_SIZE];
    const size_t n = ctrl_bin_packet_encode(&packet, tx_buffer, sizeof(tx_buffer));
    if (n == 0) {
        ctrl_stdio_print_text("[SLAVE][ERR] Nu am putut encoda răspunsul\n");
        return;
    }
    dd_uart_write_bytes(&g_uart, tx_buffer, n);
}

void build_nack(uint8_t requester_id,
                uint8_t seq,
                uint8_t reason,
                bin_packet_t* out) {
    out->id         = g_device_id;
    out->seq        = seq;
    out->cmd        = CMD_NACK;
    out->length     = 2;
    out->payload[0] = requester_id;
    out->payload[1] = reason;
}

void build_pong(uint8_t seq, bin_packet_t* out) {
    out->id     = g_device_id;
    out->seq    = seq;
    out->cmd    = CMD_PONG;
    out->length = 4;
    const uint32_t t = millis();
    out->payload[0] = static_cast<uint8_t>((t >> 24) & 0xFF);
    out->payload[1] = static_cast<uint8_t>((t >> 16) & 0xFF);
    out->payload[2] = static_cast<uint8_t>((t >> 8)  & 0xFF);
    out->payload[3] = static_cast<uint8_t>(t         & 0xFF);
}

void build_counter_value(uint8_t seq, bin_packet_t* out) {
    out->id     = g_device_id;
    out->seq    = seq;
    out->cmd    = CMD_COUNTER_VALUE;
    out->length = 4;
    const uint32_t c = g_app_counter;
    out->payload[0] = static_cast<uint8_t>((c >> 24) & 0xFF);
    out->payload[1] = static_cast<uint8_t>((c >> 16) & 0xFF);
    out->payload[2] = static_cast<uint8_t>((c >> 8)  & 0xFF);
    out->payload[3] = static_cast<uint8_t>(c         & 0xFF);
}

void process_frame(const fifo_slot_t& slot) {
    bin_packet_t in;
    const bin_decode_result_t r =
        ctrl_bin_packet_decode(slot.frame, slot.length, &in);

    // Extragere "best effort" a SEQ-ului pentru NACK chiar și la checksum greșit
    const uint8_t requester_id =
        (slot.length >= 2) ? slot.frame[1] : DEVICE_ID_BROADCAST;
    const uint8_t requester_seq =
        (slot.length >= 3) ? slot.frame[2] : 0;

    if (r == BIN_DECODE_BAD_CHECKSUM) {
        ++g_stat_bad_checksum;
        ctrl_stdio_printf(
            "[SLAVE][NACK] SEQ=%u — checksum invalid\n",
            static_cast<unsigned>(requester_seq));
        bin_packet_t nack;
        build_nack(requester_id, requester_seq, NACK_REASON_BAD_CHECKSUM, &nack);
        send_response(nack);
        return;
    }
    if (r != BIN_DECODE_OK) {
        ctrl_stdio_printf("[SLAVE][NACK] SEQ=%u — structură invalidă (r=%u)\n",
                          static_cast<unsigned>(requester_seq),
                          static_cast<unsigned>(r));
        bin_packet_t nack;
        build_nack(requester_id, requester_seq, NACK_REASON_BAD_LENGTH, &nack);
        send_response(nack);
        return;
    }

    // Filtru pe ID: acceptăm doar pachete adresate nouă sau broadcast.
    if (in.id != g_device_id && in.id != DEVICE_ID_BROADCAST) {
        ++g_stat_bad_id;
        ctrl_stdio_printf(
            "[SLAVE][IGN] Pachet pentru ID=0x%02X (nu sunt eu)\n",
            static_cast<unsigned>(in.id));
        return;
    }

    // Dedup (bonus): dacă master-ul retransmite, îl recunoaștem după (ID, SEQ).
    if (dedup_seen(requester_id, in.seq)) {
        ++g_stat_duplicates;
        ctrl_stdio_printf(
            "[SLAVE][DUP ] SEQ=%u — retransmisie, reconfirm răspunsul\n",
            static_cast<unsigned>(in.seq));
    } else {
        dedup_record(requester_id, in.seq);
        ++g_stat_accepted;
        ++g_app_counter;
    }

    bin_packet_t response;
    switch (in.cmd) {
        case CMD_PING:
            build_pong(in.seq, &response);
            ctrl_stdio_printf(
                "[SLAVE][ RX ] PING SEQ=%u  => PONG (uptime 4B)\n",
                static_cast<unsigned>(in.seq));
            send_response(response);
            break;
        case CMD_GET_COUNTER:
            build_counter_value(in.seq, &response);
            ctrl_stdio_printf(
                "[SLAVE][ RX ] GET_COUNTER SEQ=%u  => cnt=%lu\n",
                static_cast<unsigned>(in.seq),
                static_cast<unsigned long>(g_app_counter));
            send_response(response);
            break;
        default:
            ctrl_stdio_printf(
                "[SLAVE][NACK] SEQ=%u — CMD=0x%02X necunoscut\n",
                static_cast<unsigned>(in.seq),
                static_cast<unsigned>(in.cmd));
            build_nack(in.id, in.seq, NACK_REASON_UNKNOWN_CMD, &response);
            send_response(response);
            break;
    }
}

void task_uart_rx(void* parameters) {
    (void)parameters;
    for (;;) {
        int b = dd_uart_read_byte(&g_uart);
        while (b >= 0) {
            size_t frame_len = 0;
            if (dd_uart_feed_byte(&g_uart,
                                  static_cast<uint8_t>(b),
                                  &frame_len)) {
                ++g_stat_received;
                fifo_slot_t slot;
                slot.length = static_cast<uint16_t>(frame_len);
                const size_t n =
                    (frame_len <= sizeof(slot.frame)) ? frame_len
                                                      : sizeof(slot.frame);
                for (size_t k = 0; k < n; ++k) {
                    slot.frame[k] = g_uart.frame[k];
                }
                if (xQueueSend(g_rx_fifo, &slot, 0) != pdTRUE) {
                    ctrl_stdio_print_text(
                        "[SLAVE][ERR] FIFO plin — pachet aruncat\n");
                }
            }
            b = dd_uart_read_byte(&g_uart);
        }
        vTaskDelay(pdMS_TO_TICKS(UART_RX_TASK_IDLE_TICK_MS));
    }
}

void task_packet_processor(void* parameters) {
    (void)parameters;
    fifo_slot_t slot;
    for (;;) {
        if (xQueueReceive(g_rx_fifo, &slot, portMAX_DELAY) == pdTRUE) {
            process_frame(slot);
        }
    }
}

void task_diag(void* parameters) {
    (void)parameters;
    TickType_t last = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(SLAVE_DIAG_PERIOD_MS);
    for (;;) {
        vTaskDelayUntil(&last, period);
        ctrl_stdio_printf(
            "[SLAVE][stats] rx=%lu acc=%lu bad_cs=%lu bad_id=%lu dup=%lu cnt=%lu\n",
            static_cast<unsigned long>(g_stat_received),
            static_cast<unsigned long>(g_stat_accepted),
            static_cast<unsigned long>(g_stat_bad_checksum),
            static_cast<unsigned long>(g_stat_bad_id),
            static_cast<unsigned long>(g_stat_duplicates),
            static_cast<unsigned long>(g_app_counter));
    }
}

}  // namespace

void app_uart_slave_init(HardwareSerial* hw,
                         unsigned long baudrate,
                         int8_t rx_pin,
                         int8_t tx_pin,
                         uint8_t device_id) {
    g_device_id = device_id;
    for (uint8_t i = 0; i < SLAVE_DEDUP_WINDOW; ++i) {
        g_dedup[i].used = false;
    }

    dd_uart_init(&g_uart, hw, baudrate, rx_pin, tx_pin);

    g_rx_fifo = xQueueCreate(SLAVE_RX_FIFO_DEPTH, sizeof(fifo_slot_t));
    if (g_rx_fifo == nullptr) {
        ctrl_stdio_print_text("[SLAVE][EROARE] Nu pot aloca FIFO\n");
        return;
    }

    const BaseType_t rx_ok = xTaskCreate(
        task_uart_rx, "uart_rx", TASK_STACK_SIZE, nullptr,
        TASK_PRIORITY_UART_RX, nullptr);
    const BaseType_t pp_ok = xTaskCreate(
        task_packet_processor, "pkt_proc", TASK_STACK_SIZE, nullptr,
        TASK_PRIORITY_PACKET_PROC, nullptr);
    const BaseType_t dg_ok = xTaskCreate(
        task_diag, "slave_diag", TASK_STACK_SIZE, nullptr,
        TASK_PRIORITY_DIAG, nullptr);

    if (rx_ok != pdPASS || pp_ok != pdPASS || dg_ok != pdPASS) {
        ctrl_stdio_print_text("[SLAVE][EROARE] Nu am putut crea task-urile\n");
        return;
    }

    if (hw != nullptr) {
        ctrl_stdio_printf(
            "[SLAVE] gata. ID=0x%02X, UART baud=%lu (RX=%d, TX=%d), FIFO=%u\n",
            static_cast<unsigned>(g_device_id),
            static_cast<unsigned long>(baudrate),
            static_cast<int>(rx_pin),
            static_cast<int>(tx_pin),
            static_cast<unsigned>(SLAVE_RX_FIFO_DEPTH));
    } else {
        ctrl_stdio_printf(
            "[SLAVE] gata (DEMO / UART virtual). ID=0x%02X, FIFO=%u\n",
            static_cast<unsigned>(g_device_id),
            static_cast<unsigned>(SLAVE_RX_FIFO_DEPTH));
    }
}

dd_uart_t* app_uart_slave_get_uart() {
    return &g_uart;
}

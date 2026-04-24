#include "ctrl_bin_packet.h"

uint8_t ctrl_bin_packet_checksum(const bin_packet_t* packet) {
    if (packet == nullptr) {
        return 0;
    }
    uint8_t cs = 0;
    cs ^= packet->id;
    cs ^= packet->seq;
    cs ^= packet->cmd;
    cs ^= packet->length;
    const uint8_t n =
        (packet->length <= PROTO_MAX_PAYLOAD_SIZE) ? packet->length
                                                   : PROTO_MAX_PAYLOAD_SIZE;
    for (uint8_t i = 0; i < n; ++i) {
        cs ^= packet->payload[i];
    }
    return cs;
}

size_t ctrl_bin_packet_encode(const bin_packet_t* packet,
                              uint8_t* out,
                              size_t capacity) {
    if (packet == nullptr || out == nullptr) {
        return 0;
    }
    if (packet->length > PROTO_MAX_PAYLOAD_SIZE) {
        return 0;
    }
    const size_t total = PROTO_FRAME_OVERHEAD_BYTES + packet->length;
    if (capacity < total) {
        return 0;
    }

    size_t i = 0;
    out[i++] = PROTO_START_BYTE;
    out[i++] = packet->id;
    out[i++] = packet->seq;
    out[i++] = packet->cmd;
    out[i++] = packet->length;
    for (uint8_t k = 0; k < packet->length; ++k) {
        out[i++] = packet->payload[k];
    }
    out[i++] = ctrl_bin_packet_checksum(packet);
    out[i++] = PROTO_END_BYTE;
    return i;
}

bin_decode_result_t ctrl_bin_packet_decode(const uint8_t* frame,
                                           size_t frame_length,
                                           bin_packet_t* out_packet) {
    if (frame == nullptr || out_packet == nullptr) {
        return BIN_DECODE_BAD_START;
    }
    if (frame_length < PROTO_FRAME_OVERHEAD_BYTES) {
        return BIN_DECODE_NEED_MORE;
    }
    if (frame[0] != PROTO_START_BYTE) {
        return BIN_DECODE_BAD_START;
    }

    const uint8_t length = frame[4];
    if (length > PROTO_MAX_PAYLOAD_SIZE) {
        return BIN_DECODE_BAD_LENGTH;
    }

    const size_t expected_size = PROTO_FRAME_OVERHEAD_BYTES + length;
    if (frame_length < expected_size) {
        return BIN_DECODE_NEED_MORE;
    }
    if (frame[expected_size - 1] != PROTO_END_BYTE) {
        return BIN_DECODE_BAD_END;
    }

    out_packet->id     = frame[1];
    out_packet->seq    = frame[2];
    out_packet->cmd    = frame[3];
    out_packet->length = length;
    for (uint8_t k = 0; k < length; ++k) {
        out_packet->payload[k] = frame[5 + k];
    }

    const uint8_t cs_received = frame[5 + length];
    const uint8_t cs_computed = ctrl_bin_packet_checksum(out_packet);
    if (cs_received != cs_computed) {
        return BIN_DECODE_BAD_CHECKSUM;
    }
    return BIN_DECODE_OK;
}

const char* ctrl_bin_packet_cmd_name(uint8_t cmd) {
    switch (cmd) {
        case CMD_PING:           return "PING";
        case CMD_PONG:           return "PONG";
        case CMD_GET_COUNTER:    return "GET_COUNTER";
        case CMD_COUNTER_VALUE:  return "COUNTER_VALUE";
        case CMD_ACK:            return "ACK";
        case CMD_NACK:           return "NACK";
        default:                 return "??";
    }
}

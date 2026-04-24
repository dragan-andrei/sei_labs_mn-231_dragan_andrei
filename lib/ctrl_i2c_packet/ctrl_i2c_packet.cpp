#include "ctrl_i2c_packet.h"

uint8_t ctrl_i2c_packet_checksum(uint8_t head,
                                 uint8_t length,
                                 const uint8_t* payload) {
    uint8_t cs = head ^ length;
    if (payload != nullptr) {
        for (uint8_t i = 0; i < length; ++i) {
            cs ^= payload[i];
        }
    }
    return cs;
}

size_t ctrl_i2c_packet_encode(const uint16_t* values,
                              uint8_t value_count,
                              uint8_t* out,
                              size_t out_capacity) {
    if (values == nullptr || out == nullptr) {
        return 0;
    }

    const uint8_t payload_len = static_cast<uint8_t>(value_count * sizeof(uint16_t));
    if (payload_len > I2C_PACKET_MAX_PAYLOAD) {
        return 0;
    }

    const size_t total = static_cast<size_t>(I2C_PACKET_OVERHEAD) + payload_len;
    if (out_capacity < total) {
        return 0;
    }

    out[0] = I2C_PACKET_HEAD;
    out[1] = payload_len;

    for (uint8_t i = 0; i < value_count; ++i) {
        const uint16_t v = values[i];
        out[2 + (i * 2)]     = static_cast<uint8_t>((v >> 8) & 0xFF); // MSB
        out[2 + (i * 2) + 1] = static_cast<uint8_t>(v & 0xFF);        // LSB
    }

    out[total - 1] = ctrl_i2c_packet_checksum(out[0], out[1], &out[2]);
    return total;
}

bool ctrl_i2c_packet_decode(const uint8_t* data,
                            size_t data_len,
                            i2c_packet_t* out_packet) {
    if (data == nullptr || out_packet == nullptr) {
        return false;
    }
    if (data_len < static_cast<size_t>(I2C_PACKET_OVERHEAD)) {
        return false;
    }
    if (data[0] != I2C_PACKET_HEAD) {
        return false;
    }

    const uint8_t payload_len = data[1];
    if (payload_len > I2C_PACKET_MAX_PAYLOAD) {
        return false;
    }
    if (data_len < static_cast<size_t>(I2C_PACKET_OVERHEAD) + payload_len) {
        return false;
    }

    const uint8_t received_cs = data[I2C_PACKET_OVERHEAD - 1 + payload_len];
    const uint8_t expected_cs = ctrl_i2c_packet_checksum(data[0], data[1], &data[2]);
    if (received_cs != expected_cs) {
        return false;
    }

    out_packet->head     = data[0];
    out_packet->length   = payload_len;
    out_packet->checksum = received_cs;
    for (uint8_t i = 0; i < payload_len; ++i) {
        out_packet->payload[i] = data[2 + i];
    }
    return true;
}

uint16_t ctrl_i2c_packet_get_value(const i2c_packet_t* packet, uint8_t index) {
    if (packet == nullptr) {
        return HCSR04_DISTANCE_ERROR;
    }
    const uint8_t offset = static_cast<uint8_t>(index * sizeof(uint16_t));
    if (offset + 1 >= packet->length) {
        return HCSR04_DISTANCE_ERROR;
    }

    const uint16_t hi = packet->payload[offset];
    const uint16_t lo = packet->payload[offset + 1];
    return static_cast<uint16_t>((hi << 8) | lo);
}

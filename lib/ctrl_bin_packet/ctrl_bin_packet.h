#ifndef CTRL_BIN_PACKET_H
#define CTRL_BIN_PACKET_H

#include <stddef.h>
#include <stdint.h>

#include "configs.h"

// Reprezentarea logică (deja validată) a unui frame
// [STX][ID][SEQ][CMD][LEN][PAYLOAD][CS][ETX].
typedef struct {
    uint8_t id;
    uint8_t seq;
    uint8_t cmd;
    uint8_t length;
    uint8_t payload[PROTO_MAX_PAYLOAD_SIZE];
} bin_packet_t;

// ============================================================================
//  Calcul checksum
// ============================================================================
//  XOR peste ID, SEQ, CMD, LEN și toți octeții din payload.
uint8_t ctrl_bin_packet_checksum(const bin_packet_t* packet);

// ============================================================================
//  Encode: serializează `packet` în bufferul `out`.
// ============================================================================
//  Returnează numărul de octeți scriși sau 0 la eroare (out nul, capacitate
//  insuficientă, length > PROTO_MAX_PAYLOAD_SIZE).
size_t ctrl_bin_packet_encode(const bin_packet_t* packet,
                              uint8_t* out,
                              size_t capacity);

// ============================================================================
//  Rezultatul unui tentativ de decodare.
// ============================================================================
typedef enum {
    BIN_DECODE_OK              = 0,
    BIN_DECODE_NEED_MORE       = 1,
    BIN_DECODE_BAD_START       = 2,
    BIN_DECODE_BAD_END         = 3,
    BIN_DECODE_BAD_LENGTH      = 4,
    BIN_DECODE_BAD_CHECKSUM    = 5,
} bin_decode_result_t;

// ============================================================================
//  Decode: parsează `frame` (exact un cadru complet) și populează `packet`.
// ============================================================================
//  Nu face resincronizare pe stream — asta o face parser-ul cu state machine
//  din `dd_uart`. Este doar validator pe un buffer deja extras.
bin_decode_result_t ctrl_bin_packet_decode(const uint8_t* frame,
                                           size_t frame_length,
                                           bin_packet_t* out_packet);

// Ajutător pentru logging
const char* ctrl_bin_packet_cmd_name(uint8_t cmd);

#endif // CTRL_BIN_PACKET_H

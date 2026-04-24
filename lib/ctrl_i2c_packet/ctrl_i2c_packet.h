#ifndef CTRL_I2C_PACKET_H
#define CTRL_I2C_PACKET_H

#include <Arduino.h>

#include "configs.h"

// Structură parsată pentru un pachet I²C valid.
// Un pachet pe magistrală are forma:
//   [HEAD] [LENGTH] [PAYLOAD ... LENGTH octeți] [CHECKSUM]
// PAYLOAD reprezintă N × uint16_t (big-endian) — câte un senzor.
typedef struct {
    uint8_t  head;
    uint8_t  length;                                  // lungime payload în octeți
    uint8_t  payload[I2C_PACKET_MAX_PAYLOAD];
    uint8_t  checksum;
} i2c_packet_t;

// Codifică N valori uint16_t într-un pachet binar gata de trimis pe magistrală.
// Returnează numărul total de octeți scriși în `out` sau 0 în caz de eroare.
// Primul octet este HEAD, urmat de LENGTH, PAYLOAD și CHECKSUM.
size_t ctrl_i2c_packet_encode(const uint16_t* values,
                              uint8_t value_count,
                              uint8_t* out,
                              size_t out_capacity);

// Decodifică un flux de octeți primit pe magistrală într-un pachet structurat.
// Verifică HEAD, LENGTH și CHECKSUM.
// Returnează true dacă pachetul este valid, false altfel.
bool ctrl_i2c_packet_decode(const uint8_t* data,
                            size_t data_len,
                            i2c_packet_t* out_packet);

// Extrage a `index`-a valoare uint16_t (big-endian) din payload.
// Întoarce HCSR04_DISTANCE_ERROR dacă indexul depășește lungimea payload-ului.
uint16_t ctrl_i2c_packet_get_value(const i2c_packet_t* packet, uint8_t index);

// Calculează checksum-ul (XOR peste HEAD, LENGTH și toți octeții PAYLOAD).
uint8_t ctrl_i2c_packet_checksum(uint8_t head,
                                 uint8_t length,
                                 const uint8_t* payload);

#endif // CTRL_I2C_PACKET_H

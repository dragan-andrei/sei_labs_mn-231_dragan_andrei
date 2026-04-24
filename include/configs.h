#ifndef CONFIGS_H
#define CONFIGS_H

#include <Arduino.h>

// ==========================================================================
// CONFIGURAȚII GENERALE (partajate Master & Slave)
// ==========================================================================
#define SERIAL_BAUDRATE                 115200

// ==========================================================================
// CONFIGURAȚII MAGISTRALĂ I²C
// ==========================================================================
#define I2C_SDA_PIN                     21       // GPIO pentru linia de date
#define I2C_SCL_PIN                     22       // GPIO pentru linia de ceas
#define I2C_BUS_CLOCK_HZ                100000   // 100 kHz (Standard Mode)
#define I2C_SLAVE_ADDRESS               0x42     // Adresa logică a nodului slave

// Cod comenzi transmise de Master către Slave (1 octet)
#define I2C_CMD_READ_ALL                0xA1     // Citește toți senzorii (pachet compact)

// Pini folosiți DOAR în build-ul `esp32_demo` (master + slave pe același chip,
// conectați prin loopback intern în diagrama Wokwi).
// Master-ul rulează pe I²C-ul 0 (Wire), slave-ul pe I²C-ul 1 (Wire1).
#define I2C_DEMO_MASTER_SDA_PIN         21
#define I2C_DEMO_MASTER_SCL_PIN         22
#define I2C_DEMO_SLAVE_SDA_PIN          16
#define I2C_DEMO_SLAVE_SCL_PIN          17

// ==========================================================================
// CONFIGURAȚII SENZOR ULTRASONIC (HC-SR04) — doar pentru Slave
// ==========================================================================
#define HCSR04_SENSOR_COUNT             2        // Număr de senzori conectați pe slave
#define HCSR04_S1_TRIG_PIN              5        // TRIG senzor #1
#define HCSR04_S1_ECHO_PIN              18       // ECHO senzor #1
#define HCSR04_S2_TRIG_PIN              19       // TRIG senzor #2
#define HCSR04_S2_ECHO_PIN              23       // ECHO senzor #2

#define HCSR04_TRIGGER_PULSE_US         10       // Durata impulsului de declanșare
#define HCSR04_ECHO_TIMEOUT_US          30000UL  // Timeout 30 ms -> ~5 m
#define HCSR04_US_PER_CM                58UL     // (343 m/s) /  dus-întors = ~58 us/cm
#define HCSR04_DISTANCE_ERROR           0xFFFF   // Cod de eroare pentru cm invalid
#define HCSR04_MAX_VALID_CM             400      // Dincolo de această limită => eroare

// ==========================================================================
// CONFIGURAȚII FORMAT PACHET I²C
//   [HEAD] [LENGTH] [PAYLOAD ... LENGTH bytes] [CHECKSUM]
//   PAYLOAD = N × 2 octeți (uint16 big-endian, câte un senzor)
// ==========================================================================
#define I2C_PACKET_HEAD                 0xAA
#define I2C_PACKET_MAX_PAYLOAD          16
#define I2C_PACKET_OVERHEAD             3        // HEAD + LENGTH + CHECKSUM
#define I2C_PACKET_MAX_SIZE             (I2C_PACKET_OVERHEAD + I2C_PACKET_MAX_PAYLOAD)

// ==========================================================================
// CONFIGURAȚII LED ALERTĂ (comportament adițional — proximitate excesivă)
// ==========================================================================
#define ALERT_LED_PIN                   2        // LED-ul integrat pe placa ESP32 DevKit
#define ALERT_PROXIMITY_CM              15       // Sub această distanță se aprinde alerta

// ==========================================================================
// CONFIGURAȚII FreeRTOS — timpi și priorități
// ==========================================================================
#define SLAVE_SAMPLE_PERIOD_MS          250      // Interval achiziție senzor
#define SLAVE_BUFFER_REFRESH_MS         250      // Interval reîmprospătare buffer I²C
#define MASTER_POLL_PERIOD_MS           500      // Interval interogare slave
#define MUTEX_BLOCK_TIMEOUT_MS          50       // Timp maxim de așteptare mutex

#define TASK_STACK_SIZE                 4096
#define TASK_PRIORITY_SAMPLER           2        // Achiziția are prioritate mai mare
#define TASK_PRIORITY_BUFFER            1
#define TASK_PRIORITY_POLLER            2
#define TASK_PRIORITY_DISPLAY           1

#endif // CONFIGS_H

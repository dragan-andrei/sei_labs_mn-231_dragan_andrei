#include "dd_hcsr04.h"

#include "configs.h"

void dd_hcsr04_init(const hcsr04_sensor_t* sensor) {
    if (sensor == nullptr) {
        return;
    }
    pinMode(sensor->trig_pin, OUTPUT);
    pinMode(sensor->echo_pin, INPUT);
    digitalWrite(sensor->trig_pin, LOW);
}

uint16_t dd_hcsr04_read_cm(const hcsr04_sensor_t* sensor) {
    if (sensor == nullptr) {
        return HCSR04_DISTANCE_ERROR;
    }

    // Impuls de declanșare conform foii de catalog: 10 us pe TRIG
    digitalWrite(sensor->trig_pin, LOW);
    delayMicroseconds(2);
    digitalWrite(sensor->trig_pin, HIGH);
    delayMicroseconds(HCSR04_TRIGGER_PULSE_US);
    digitalWrite(sensor->trig_pin, LOW);

    // pulseIn() cu timeout pentru a nu bloca task-ul FreeRTOS
    const unsigned long duration_us =
        pulseIn(sensor->echo_pin, HIGH, HCSR04_ECHO_TIMEOUT_US);

    if (duration_us == 0UL) {
        return HCSR04_DISTANCE_ERROR;
    }

    const unsigned long distance_cm = duration_us / HCSR04_US_PER_CM;
    if (distance_cm == 0UL || distance_cm > HCSR04_MAX_VALID_CM) {
        return HCSR04_DISTANCE_ERROR;
    }

    return static_cast<uint16_t>(distance_cm);
}

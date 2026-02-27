#ifndef DD_BUTTON_H
#define DD_BUTTON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define DD_BUTTON_INPUT 0x00

#define DD_BUTTON_INPUT_PULLUP 0x02

typedef struct {
    uint8_t pin;
    uint8_t state;
    uint8_t(*read_state)(uint8_t pin);
    void(*set_pin_mode)(uint8_t pin, uint8_t mode);

} dd_button_t;

void dd_button_init(
                    dd_button_t* button, uint8_t pin, uint8_t mode,
                    uint8_t(*read_state)(uint8_t), 
                    void(*set_pin_mode)(uint8_t, uint8_t)
                    );

bool dd_button_is_pressed(dd_button_t* button);

#endif // DD_BUTTON_H
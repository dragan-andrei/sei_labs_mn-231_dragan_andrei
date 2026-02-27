#include "dd_button.h"

void dd_button_init(
                    dd_button_t* button, uint8_t pin, uint8_t mode,
                    uint8_t(*read_state)(uint8_t),
                    void(*set_pin_mode)(uint8_t, uint8_t)
                    )
{
    if (button == NULL || read_state == NULL || set_pin_mode == NULL) {
        return;
    }

    button->pin = pin;
    button->state = 0;
    button->read_state = read_state;
    button->set_pin_mode = set_pin_mode;

    button->set_pin_mode(button->pin, mode);
}

bool dd_button_is_pressed(dd_button_t* button)
{
    if (button == NULL || button->read_state == NULL) {
        return 0;
    }

    button->state = button->read_state(button->pin) == 0 ? 0 : 1;

    return (button->state == 0);
}

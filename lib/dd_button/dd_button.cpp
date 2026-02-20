#include "dd_button.h"

#ifndef KEYPAD_PIN_1
#define KEYPAD_PIN_1 2
#endif

#ifndef DD_LED_ON
#define DD_LED_ON 1
#endif

#ifndef DD_LED_OFF
#define DD_LED_OFF 0
#endif

static uint8_t first_task_led_state = DD_LED_OFF;

void ctrl_button_led_task_init(
                    dd_button_t* button, uint8_t pin, uint8_t mode,
                    uint8_t(*read_state)(uint8_t), 
                    void(*set_pin_mode)(uint8_t, uint8_t)
                    )
{
    if (button == NULL || read_state == NULL || set_pin_mode == NULL) {
        return; // Handle null pointers gracefully
    }

    button->pin = pin;
    button->state = 0;
    button->read_state = read_state;
    button->set_pin_mode = set_pin_mode;

    // Set the pin mode for the button
    button->set_pin_mode(button->pin, mode);
}

void ctrl_button_led_task(void *params)
{
    static uint8_t need_init = true;
    static dd_button_t button;

    if(need_init) {
        ctrl_button_led_task_init(&button, KEYPAD_PIN_1, 0, NULL, NULL); // You should replace NULL with actual function pointers                                                                                                                                                                                                                                                                                                                           
        need_init = false;
    }
    if (dd_button_is_pressed(&button)) {
        first_task_led_state = DD_LED_ON;
    } else {
        first_task_led_state = DD_LED_OFF;
    }
}

bool dd_button_is_pressed(dd_button_t* button)
{
    if (button == NULL || button->read_state == NULL) {
        return 0; // Handle null pointers gracefully
    }

    // Read the current state of the button
    button->state = button->read_state(button->pin) == 0 ? 0 : 1; // Assuming active LOW button

    // Return true if the button is pressed (assuming active LOW)
    return (button->state == 0);
}



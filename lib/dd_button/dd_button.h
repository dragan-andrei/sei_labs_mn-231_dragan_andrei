#ifndef DD_BUTTON_H
#define DD_BUTTON_H

#include <Arduino.h>

// Inițializează pinul butonului cu rezistența internă de pull-up
void dd_button_init(uint8_t pin);

// Verifică starea butonului
bool dd_button_is_pressed(uint8_t pin);

#endif // DD_BUTTON_H
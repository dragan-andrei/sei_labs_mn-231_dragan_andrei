#ifndef DD_BUTTON_H
#define DD_BUTTON_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Structura pentru a incapsula starea de debounce
typedef struct {
    uint8_t pin;
    uint32_t last_debounce_time;
    bool last_read_state;
    bool stable_state;
    uint32_t debounce_delay_ms;
} dd_button_t;

// Inițializează structura și hardware-ul butonului
void dd_button_init(dd_button_t* btn, uint8_t pin, uint32_t debounce_ms);

// Citește starea RAW a butonului
bool dd_button_is_pressed_raw(uint8_t pin);

// Funcție care aplică debounce și returnează TRUE la un EVENIMENT (tranziție validă de apăsare)
bool dd_button_update(dd_button_t* btn, uint32_t current_time_ms);

#ifdef __cplusplus
}
#endif

#endif // DD_BUTTON_H
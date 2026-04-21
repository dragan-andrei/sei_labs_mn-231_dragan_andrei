#include "dd_relay.h"

void dd_relay_init(dd_relay_t *relay,
                   uint8_t pin_number,
                   uint8_t mode,
                   void (*set_state)(uint8_t, uint8_t),
                   void (*set_pin_mode)(uint8_t, uint8_t))
{
    if (relay == NULL || set_state == NULL || set_pin_mode == NULL)
    {
        return;
    }

    relay->pin_number = pin_number;
    relay->state = DD_RELAY_OFF;
    relay->set_state = set_state;
    relay->set_pin_mode = set_pin_mode;

    relay->set_pin_mode(relay->pin_number, mode);
    relay->set_state(relay->pin_number, LOW);
}

void dd_relay_on(dd_relay_t *relay)
{
    if (relay == NULL || relay->set_state == NULL)
    {
        return;
    }

    relay->set_state(relay->pin_number, HIGH);
    relay->state = DD_RELAY_ON;
}

void dd_relay_off(dd_relay_t *relay)
{
    if (relay == NULL || relay->set_state == NULL)
    {
        return;
    }

    relay->set_state(relay->pin_number, LOW);
    relay->state = DD_RELAY_OFF;
}

dd_relay_state_t dd_relay_get_state(const dd_relay_t *relay)
{
    if (relay == NULL)
    {
        return DD_RELAY_OFF;
    }

    return relay->state;
}

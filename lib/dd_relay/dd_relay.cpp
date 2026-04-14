#include "dd_relay.h"

static uint8_t dd_relay_inactive_signal_level(const dd_relay_t *relay)
{
    return (relay->active_signal_level == LOW) ? HIGH : LOW;
}

static void dd_relay_apply_state(dd_relay_t *relay, dd_relay_state_t state)
{
    if (relay == NULL || relay->set_state == NULL)
    {
        return;
    }

    const uint8_t signal_level = (state == DD_RELAY_ON)
                                     ? relay->active_signal_level
                                     : dd_relay_inactive_signal_level(relay);

    relay->set_state(relay->pin_number, signal_level);
    relay->state = state;
}

void dd_relay_init(dd_relay_t *relay,
                   uint8_t pin_number,
                   uint8_t mode,
                   uint8_t active_signal_level,
                   void (*set_state)(uint8_t, uint8_t),
                   void (*set_pin_mode)(uint8_t, uint8_t))
{
    if (relay == NULL || set_state == NULL || set_pin_mode == NULL)
    {
        return;
    }

    relay->pin_number = pin_number;
    relay->active_signal_level = active_signal_level;
    relay->set_state = set_state;
    relay->set_pin_mode = set_pin_mode;

    relay->set_pin_mode(relay->pin_number, mode);
    dd_relay_apply_state(relay, DD_RELAY_OFF);
}

void dd_relay_set_on(dd_relay_t *relay)
{
    dd_relay_apply_state(relay, DD_RELAY_ON);
}

void dd_relay_set_off(dd_relay_t *relay)
{
    dd_relay_apply_state(relay, DD_RELAY_OFF);
}

void dd_relay_toggle(dd_relay_t *relay)
{
    if (relay == NULL)
    {
        return;
    }

    if (relay->state == DD_RELAY_ON)
    {
        dd_relay_set_off(relay);
    }
    else
    {
        dd_relay_set_on(relay);
    }
}

dd_relay_state_t dd_relay_get_state(const dd_relay_t *relay)
{
    if (relay == NULL)
    {
        return DD_RELAY_OFF;
    }

    return relay->state;
}

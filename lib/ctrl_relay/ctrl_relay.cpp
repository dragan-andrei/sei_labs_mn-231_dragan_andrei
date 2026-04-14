#include "ctrl_relay.h"

#include <Arduino_FreeRTOS.h>
#include <task.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "configs.h"

static dd_relay_t g_relay;
static volatile uint8_t g_relay_module_initialized = 0U;
static volatile dd_relay_state_t g_relay_state = DD_RELAY_OFF;

static const char *ctrl_relay_state_to_text(dd_relay_state_t state)
{
    return (state == DD_RELAY_ON) ? "ON" : "OFF";
}

static const char *ctrl_relay_source_to_text(ctrl_relay_command_source_t source)
{
    return (source == CTRL_RELAY_COMMAND_SOURCE_KEYPAD) ? "KEYPAD" : "SERIAL";
}

static void ctrl_relay_normalize_serial_command(const char *input,
                                                char *normalized,
                                                size_t normalized_size)
{
    size_t input_idx = 0U;
    size_t normalized_idx = 0U;

    if (normalized_size == 0U)
    {
        return;
    }

    while (input != NULL && input[input_idx] != '\0' && isspace((unsigned char)input[input_idx]) != 0)
    {
        input_idx++;
    }

    while (input != NULL && input[input_idx] != '\0' && normalized_idx < (normalized_size - 1U))
    {
        normalized[normalized_idx] = (char)tolower((unsigned char)input[input_idx]);
        normalized_idx++;
        input_idx++;
    }

    while (normalized_idx > 0U && isspace((unsigned char)normalized[normalized_idx - 1U]) != 0)
    {
        normalized_idx--;
    }

    normalized[normalized_idx] = '\0';
}

void ctrl_relay_init(void)
{
    if (g_relay_module_initialized != 0U)
    {
        return;
    }

    dd_relay_init(&g_relay,
                  RELAY_PIN,
                  DD_RELAY_OUTPUT,
                  RELAY_SIGNAL_ACTIVE,
                  digitalWrite,
                  pinMode);

    taskENTER_CRITICAL();
    g_relay_state = dd_relay_get_state(&g_relay);
    g_relay_module_initialized = 1U;
    taskEXIT_CRITICAL();
}

ctrl_relay_command_t ctrl_relay_parse_keypad_command(char key)
{
    const char normalized_key = (char)toupper((unsigned char)key);

    if (normalized_key == RELAY_KEYPAD_COMMAND_ON)
    {
        return CTRL_RELAY_COMMAND_ON;
    }

    if (normalized_key == RELAY_KEYPAD_COMMAND_OFF)
    {
        return CTRL_RELAY_COMMAND_OFF;
    }

    if (normalized_key == RELAY_KEYPAD_COMMAND_TOGGLE)
    {
        return CTRL_RELAY_COMMAND_TOGGLE;
    }

    if (normalized_key == RELAY_KEYPAD_COMMAND_STATUS)
    {
        return CTRL_RELAY_COMMAND_STATUS;
    }

    return CTRL_RELAY_COMMAND_INVALID;
}

ctrl_relay_command_t ctrl_relay_parse_serial_command(const char *input)
{
    char normalized_input[RELAY_SERIAL_INPUT_BUFFER_SIZE];

    ctrl_relay_normalize_serial_command(input,
                                        normalized_input,
                                        sizeof(normalized_input));

    if (strcmp(normalized_input, RELAY_SERIAL_COMMAND_ON) == 0)
    {
        return CTRL_RELAY_COMMAND_ON;
    }

    if (strcmp(normalized_input, RELAY_SERIAL_COMMAND_OFF) == 0)
    {
        return CTRL_RELAY_COMMAND_OFF;
    }

    if (strcmp(normalized_input, RELAY_SERIAL_COMMAND_TOGGLE) == 0)
    {
        return CTRL_RELAY_COMMAND_TOGGLE;
    }

    if (strcmp(normalized_input, RELAY_SERIAL_COMMAND_STATUS) == 0)
    {
        return CTRL_RELAY_COMMAND_STATUS;
    }

    return CTRL_RELAY_COMMAND_INVALID;
}

ctrl_relay_result_t ctrl_relay_execute_command(ctrl_relay_command_t command,
                                               ctrl_relay_command_source_t source)
{
    dd_relay_state_t new_state = DD_RELAY_OFF;

    ctrl_relay_init();

    switch (command)
    {
        case CTRL_RELAY_COMMAND_ON:
            dd_relay_set_on(&g_relay);
            break;

        case CTRL_RELAY_COMMAND_OFF:
            dd_relay_set_off(&g_relay);
            break;

        case CTRL_RELAY_COMMAND_TOGGLE:
            dd_relay_toggle(&g_relay);
            break;

        case CTRL_RELAY_COMMAND_STATUS:
            break;

        default:
            printf("[RELAY][%s] WARNING: Invalid command. Use %s / %s / %s / %s\n",
                   ctrl_relay_source_to_text(source),
                   RELAY_SERIAL_COMMAND_ON,
                   RELAY_SERIAL_COMMAND_OFF,
                   RELAY_SERIAL_COMMAND_TOGGLE,
                   RELAY_SERIAL_COMMAND_STATUS);
            return CTRL_RELAY_RESULT_INVALID_COMMAND;
    }

    new_state = dd_relay_get_state(&g_relay);

    taskENTER_CRITICAL();
    g_relay_state = new_state;
    taskEXIT_CRITICAL();

    printf("[RELAY][%s] CMD=%s | STATE=%s\n",
           ctrl_relay_source_to_text(source),
           ctrl_relay_command_to_text(command),
           ctrl_relay_state_to_text(new_state));

    return CTRL_RELAY_RESULT_OK;
}

dd_relay_state_t ctrl_relay_get_state(void)
{
    dd_relay_state_t state_snapshot;

    taskENTER_CRITICAL();
    state_snapshot = g_relay_state;
    taskEXIT_CRITICAL();

    return state_snapshot;
}

void ctrl_relay_get_status_text(char *buffer, size_t buffer_size)
{
    const dd_relay_state_t state_snapshot = ctrl_relay_get_state();

    if (buffer == NULL || buffer_size == 0U)
    {
        return;
    }

    snprintf(buffer,
             buffer_size,
             "Relay: %s",
             ctrl_relay_state_to_text(state_snapshot));
}

const char *ctrl_relay_command_to_text(ctrl_relay_command_t command)
{
    switch (command)
    {
        case CTRL_RELAY_COMMAND_ON:
            return "ON";

        case CTRL_RELAY_COMMAND_OFF:
            return "OFF";

        case CTRL_RELAY_COMMAND_TOGGLE:
            return "TOGGLE";

        case CTRL_RELAY_COMMAND_STATUS:
            return "STATUS";

        default:
            return "INVALID";
    }
}

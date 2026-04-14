#include "ctrl_task_relay.h"

#include <Arduino_FreeRTOS.h>
#include <task.h>

#include <stdio.h>

#include "configs.h"
#include "ctrl_relay.h"
#include "ctrl_stdio.h"

static void ctrl_relay_update_lcd_status(void)
{
    static uint8_t is_lcd_initialized = 0U;
    static dd_relay_state_t previous_state = DD_RELAY_OFF;
    const dd_relay_state_t current_state = ctrl_relay_get_state();
    char status_line[RELAY_STATUS_TEXT_BUFFER_SIZE];

    if (is_lcd_initialized != 0U && current_state == previous_state)
    {
        return;
    }

    ctrl_relay_get_status_text(status_line, sizeof(status_line));
    ctrl_stdio_lcd_print_two_lines("Relay Control", status_line);

    previous_state = current_state;
    is_lcd_initialized = 1U;
}

static void ctrl_relay_process_keypad_input(void)
{
    char keypad_key = '\0';

    if (ctrl_stdio_keypad_read_char(&keypad_key) == false)
    {
        return;
    }

    const ctrl_relay_command_t command = ctrl_relay_parse_keypad_command(keypad_key);

    if (command == CTRL_RELAY_COMMAND_INVALID)
    {
        printf("[RELAY][KEYPAD] WARNING: Invalid key '%c'. Use %c/%c/%c/%c\n",
               keypad_key,
               RELAY_KEYPAD_COMMAND_ON,
               RELAY_KEYPAD_COMMAND_OFF,
               RELAY_KEYPAD_COMMAND_TOGGLE,
               RELAY_KEYPAD_COMMAND_STATUS);
        return;
    }

    (void)ctrl_relay_execute_command(command, CTRL_RELAY_COMMAND_SOURCE_KEYPAD);
}

static void ctrl_relay_process_serial_line(const char *serial_line)
{
    const ctrl_relay_command_t command = ctrl_relay_parse_serial_command(serial_line);

    if (command == CTRL_RELAY_COMMAND_INVALID)
    {
        printf("[RELAY][SERIAL] INPUT='%s'\n", serial_line);
    }

    (void)ctrl_relay_execute_command(command, CTRL_RELAY_COMMAND_SOURCE_SERIAL);
}

static void ctrl_relay_finalize_serial_command(char *serial_buffer, size_t *serial_buffer_index)
{
    if (serial_buffer == NULL || serial_buffer_index == NULL)
    {
        return;
    }

    serial_buffer[*serial_buffer_index] = '\0';

    if (*serial_buffer_index > 0U)
    {
        ctrl_relay_process_serial_line(serial_buffer);
    }

    *serial_buffer_index = 0U;
}

static void ctrl_relay_process_serial_input(void)
{
    static char serial_buffer[RELAY_SERIAL_INPUT_BUFFER_SIZE];
    static size_t serial_buffer_index = 0U;
    char serial_character = '\0';

    while (ctrl_stdio_serial_read_char(&serial_character))
    {
        if (serial_character == '\r' || serial_character == '\n' || serial_character == ';')
        {
            ctrl_relay_finalize_serial_command(serial_buffer, &serial_buffer_index);
            continue;
        }

        if (serial_character == '\b' || serial_character == 127)
        {
            if (serial_buffer_index > 0U)
            {
                serial_buffer_index--;
            }

            continue;
        }

        if (serial_buffer_index < (sizeof(serial_buffer) - 1U))
        {
            serial_buffer[serial_buffer_index] = serial_character;
            serial_buffer_index++;
        }
        else
        {
            serial_buffer_index = 0U;
            printf("[RELAY][SERIAL] WARNING: Command too long. Max=%u chars\n",
                   (unsigned int)(sizeof(serial_buffer) - 1U));
        }
    }
}

void ctrl_relay_task_init(void)
{
    ctrl_relay_init();
    ctrl_relay_update_lcd_status();

    printf("[RELAY] Ready. Keypad %c/%c/%c/%c, serial: '%s'|'%s'|'%s'|'%s'\n",
           RELAY_KEYPAD_COMMAND_ON,
           RELAY_KEYPAD_COMMAND_OFF,
           RELAY_KEYPAD_COMMAND_TOGGLE,
           RELAY_KEYPAD_COMMAND_STATUS,
           RELAY_SERIAL_COMMAND_ON,
           RELAY_SERIAL_COMMAND_OFF,
           RELAY_SERIAL_COMMAND_TOGGLE,
           RELAY_SERIAL_COMMAND_STATUS);
}

void ctrl_relay_command_task(void *params)
{
    (void)params;

    vTaskDelay(pdMS_TO_TICKS(RELAY_COMMAND_TASK_OFFSET_MS));

    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        ctrl_relay_process_keypad_input();
        ctrl_relay_process_serial_input();
        ctrl_relay_update_lcd_status();

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(RELAY_COMMAND_TASK_RECURRANCE_MS));
    }
}

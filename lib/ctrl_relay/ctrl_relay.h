#ifndef ctrl_relay_h
#define ctrl_relay_h

#include <stddef.h>

#include "dd_relay.h"

typedef enum
{
    CTRL_RELAY_COMMAND_INVALID = 0,
    CTRL_RELAY_COMMAND_ON,
    CTRL_RELAY_COMMAND_OFF,
    CTRL_RELAY_COMMAND_TOGGLE,
    CTRL_RELAY_COMMAND_STATUS
} ctrl_relay_command_t;

typedef enum
{
    CTRL_RELAY_COMMAND_SOURCE_KEYPAD = 0,
    CTRL_RELAY_COMMAND_SOURCE_SERIAL
} ctrl_relay_command_source_t;

typedef enum
{
    CTRL_RELAY_RESULT_OK = 0,
    CTRL_RELAY_RESULT_INVALID_COMMAND
} ctrl_relay_result_t;

void ctrl_relay_init(void);
ctrl_relay_command_t ctrl_relay_parse_keypad_command(char key);
ctrl_relay_command_t ctrl_relay_parse_serial_command(const char *input);
ctrl_relay_result_t ctrl_relay_execute_command(ctrl_relay_command_t command,
                                               ctrl_relay_command_source_t source);
dd_relay_state_t ctrl_relay_get_state(void);
void ctrl_relay_get_status_text(char *buffer, size_t buffer_size);
const char *ctrl_relay_command_to_text(ctrl_relay_command_t command);

#endif
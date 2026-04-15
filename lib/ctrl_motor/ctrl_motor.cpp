#include "ctrl_motor.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "configs.h"
#include "ctrl_stdio.h"

static dd_motor_t g_motor;
static volatile int8_t g_target_power_percent = MOTOR_POWER_DEFAULT_PERCENT;
static volatile int8_t g_applied_power_percent = MOTOR_POWER_DEFAULT_PERCENT;
static volatile uint8_t g_motor_initialized = 0U;
static volatile uint8_t g_status_changed = 1U;
static volatile uint8_t g_limit_event = (uint8_t)CTRL_MOTOR_LIMIT_NONE;

static char g_serial_command_buffer[MOTOR_SERIAL_INPUT_BUFFER_SIZE];
static uint8_t g_serial_command_length = 0U;

static void ctrl_motor_print_serial_prompt(void)
{
    printf("> ");
}

static int8_t ctrl_motor_clamp_power_percent(int16_t power_percent)
{
    if (power_percent > MOTOR_POWER_MAX_PERCENT)
    {
        return MOTOR_POWER_MAX_PERCENT;
    }

    if (power_percent < MOTOR_POWER_MIN_PERCENT)
    {
        return MOTOR_POWER_MIN_PERCENT;
    }

    return (int8_t)power_percent;
}

static const char *ctrl_motor_direction_to_text(dd_motor_direction_t direction)
{
    switch (direction)
    {
        case DD_MOTOR_DIRECTION_FORWARD:
            return "FWD";

        case DD_MOTOR_DIRECTION_BACKWARD:
            return "REV";

        case DD_MOTOR_DIRECTION_STOP:
        default:
            return "STOP";
    }
}

static void ctrl_motor_normalize_serial_line(const char *input,
                                             char *output,
                                             size_t output_size)
{
    size_t read_index = 0U;
    size_t write_index = 0U;

    if (input == NULL || output == NULL || output_size == 0U)
    {
        return;
    }

    while (input[read_index] != '\0' && isspace((unsigned char)input[read_index]) != 0)
    {
        read_index++;
    }

    while (input[read_index] != '\0' && write_index < (output_size - 1U))
    {
        output[write_index] = (char)tolower((unsigned char)input[read_index]);
        write_index++;
        read_index++;
    }

    output[write_index] = '\0';

    while (write_index > 0U && isspace((unsigned char)output[write_index - 1U]) != 0)
    {
        output[write_index - 1U] = '\0';
        write_index--;
    }
}

static void ctrl_motor_mark_limit_event(int8_t previous_target, int8_t new_target)
{
#if (MOTOR_LIMIT_MESSAGE_ENABLED == 1)
    if (new_target == MOTOR_POWER_MAX_PERCENT && previous_target != MOTOR_POWER_MAX_PERCENT)
    {
        g_limit_event = (uint8_t)CTRL_MOTOR_LIMIT_MAX;
    }
    else if (new_target == MOTOR_POWER_MIN_PERCENT && previous_target != MOTOR_POWER_MIN_PERCENT)
    {
        g_limit_event = (uint8_t)CTRL_MOTOR_LIMIT_MIN;
    }
#else
    (void)previous_target;
    (void)new_target;
#endif
}

static int8_t ctrl_motor_request_power_percent(int16_t requested_power_percent, const char *source)
{
    const int8_t previous_target = g_target_power_percent;
    const int8_t clamped_target = ctrl_motor_clamp_power_percent(requested_power_percent);

    g_target_power_percent = clamped_target;

    if (previous_target != clamped_target)
    {
        g_status_changed = 1U;
    }

    ctrl_motor_mark_limit_event(previous_target, clamped_target);

    printf("[MOTOR][%s] target=%d%%\n", source, (int)clamped_target);

    return clamped_target;
}

static void ctrl_motor_command_stop(const char *source)
{
    (void)ctrl_motor_request_power_percent(MOTOR_POWER_DEFAULT_PERCENT, source);
}

static void ctrl_motor_command_max(const char *source)
{
    int16_t target_power = MOTOR_POWER_MAX_PERCENT;

    if (g_target_power_percent < 0)
    {
        target_power = MOTOR_POWER_MIN_PERCENT;
    }

    (void)ctrl_motor_request_power_percent(target_power, source);
}

static void ctrl_motor_command_inc(const char *source)
{
    int16_t target_power = g_target_power_percent;

    if (target_power >= 0)
    {
        target_power += MOTOR_POWER_STEP_PERCENT;
    }
    else
    {
        target_power -= MOTOR_POWER_STEP_PERCENT;
    }

    (void)ctrl_motor_request_power_percent(target_power, source);
}

static void ctrl_motor_command_dec(const char *source)
{
    int16_t target_power = g_target_power_percent;

    if (target_power > 0)
    {
        target_power -= MOTOR_POWER_STEP_PERCENT;

        if (target_power < 0)
        {
            target_power = 0;
        }
    }
    else if (target_power < 0)
    {
        target_power += MOTOR_POWER_STEP_PERCENT;

        if (target_power > 0)
        {
            target_power = 0;
        }
    }

    (void)ctrl_motor_request_power_percent(target_power, source);
}

static void ctrl_motor_execute_serial_command(const char *raw_line)
{
    char command_line[MOTOR_SERIAL_INPUT_BUFFER_SIZE];
    int requested_power = 0;

    ctrl_motor_normalize_serial_line(raw_line, command_line, sizeof(command_line));

    if (command_line[0] == '\0')
    {
        return;
    }

    if (sscanf(command_line, "motor set %d", &requested_power) == 1)
    {
        const int8_t applied_target = ctrl_motor_request_power_percent((int16_t)requested_power, "SERIAL");

        if ((int16_t)applied_target != requested_power)
        {
            printf("[MOTOR][SERIAL] clamped to %d%%\n", (int)applied_target);
        }

        return;
    }

    if (strcmp(command_line, MOTOR_SERIAL_COMMAND_STOP) == 0)
    {
        ctrl_motor_command_stop("SERIAL");
        return;
    }

    if (strcmp(command_line, MOTOR_SERIAL_COMMAND_MAX) == 0)
    {
        ctrl_motor_command_max("SERIAL");
        return;
    }

    if (strcmp(command_line, MOTOR_SERIAL_COMMAND_INC) == 0)
    {
        ctrl_motor_command_inc("SERIAL");
        return;
    }

    if (strcmp(command_line, MOTOR_SERIAL_COMMAND_DEC) == 0)
    {
        ctrl_motor_command_dec("SERIAL");
        return;
    }

    printf("[MOTOR][SERIAL] unknown command: %s\n", command_line);
    ctrl_motor_print_command_help();
}

void ctrl_motor_init(void)
{
    if (g_motor_initialized != 0U)
    {
        return;
    }

    dd_motor_init(&g_motor,
                  MOTOR_L298_ENA_PWM_PIN,
                  MOTOR_L298_IN1_PIN,
                  MOTOR_L298_IN2_PIN,
                  MOTOR_PWM_MIN,
                  MOTOR_PWM_MAX,
                  digitalWrite,
                  analogWrite,
                  pinMode);

    g_target_power_percent = MOTOR_POWER_DEFAULT_PERCENT;
    g_applied_power_percent = MOTOR_POWER_DEFAULT_PERCENT;
    g_status_changed = 1U;
    g_limit_event = (uint8_t)CTRL_MOTOR_LIMIT_NONE;
    g_serial_command_length = 0U;
    g_serial_command_buffer[0] = '\0';
    g_motor_initialized = 1U;

    printf("[MOTOR] initialized\n");
    ctrl_motor_print_command_help();
    ctrl_motor_print_serial_prompt();
}

void ctrl_motor_print_command_help(void)
{
    printf("[MOTOR] Commands: motor set [-100..100], motor stop, motor max, motor inc, motor dec\n");
    printf("[MOTOR] Keypad: A=max, B=stop, C=inc, D=dec\n");
}

void ctrl_motor_handle_serial_char(char character)
{
    if (g_motor_initialized == 0U)
    {
        return;
    }

    if (character == '\r' || character == '\n')
    {
        if (g_serial_command_length > 0U)
        {
            printf("\n");
            g_serial_command_buffer[g_serial_command_length] = '\0';
            ctrl_motor_execute_serial_command(g_serial_command_buffer);
            g_serial_command_length = 0U;
            g_serial_command_buffer[0] = '\0';
        }

        ctrl_motor_print_serial_prompt();

        return;
    }

    if (character == '\b' || character == 127)
    {
        if (g_serial_command_length > 0U)
        {
            g_serial_command_length--;
            g_serial_command_buffer[g_serial_command_length] = '\0';
            printf("\b \b");
        }

        return;
    }

    if (isprint((unsigned char)character) == 0)
    {
        return;
    }

    if (g_serial_command_length < (uint8_t)(sizeof(g_serial_command_buffer) - 1U))
    {
        g_serial_command_buffer[g_serial_command_length] = character;
        g_serial_command_length++;
        g_serial_command_buffer[g_serial_command_length] = '\0';
        printf("%c", character);
    }
}

void ctrl_motor_handle_keypad_char(char key)
{
    const char normalized_key = (char)toupper((unsigned char)key);

    if (g_motor_initialized == 0U)
    {
        return;
    }

    printf("\n[MOTOR][KEYPAD] key=%c\n", normalized_key);
    ctrl_motor_print_serial_prompt();

    if (normalized_key == MOTOR_KEYPAD_COMMAND_MAX)
    {
        ctrl_motor_command_max("KEYPAD");
    }
    else if (normalized_key == MOTOR_KEYPAD_COMMAND_STOP)
    {
        ctrl_motor_command_stop("KEYPAD");
    }
    else if (normalized_key == MOTOR_KEYPAD_COMMAND_INC)
    {
        ctrl_motor_command_inc("KEYPAD");
    }
    else if (normalized_key == MOTOR_KEYPAD_COMMAND_DEC)
    {
        ctrl_motor_command_dec("KEYPAD");
    }
    else
    {
        printf("[MOTOR][KEYPAD] unsupported key. Use A/B/C/D\n");
    }
}

void ctrl_motor_control_step(void)
{
    const int8_t target_power = g_target_power_percent;

    if (g_motor_initialized == 0U)
    {
        return;
    }

    if (target_power == g_applied_power_percent)
    {
        return;
    }

    dd_motor_set_power_percent(&g_motor, target_power);
    g_applied_power_percent = target_power;
    g_status_changed = 1U;
}

void ctrl_motor_get_status(ctrl_motor_status_t *status)
{
    if (status == NULL)
    {
        return;
    }

    status->target_power_percent = g_target_power_percent;
    status->applied_power_percent = g_applied_power_percent;
    status->motor_state = dd_motor_get_state(&g_motor);
    status->limit_event = (ctrl_motor_limit_event_t)g_limit_event;
}

void ctrl_motor_report_status_if_due(void)
{
    static uint32_t last_report_time = 0U;
    const uint32_t now = millis();
    const uint8_t periodic_due = (uint8_t)((now - last_report_time) >= MOTOR_REPORT_PERIOD_MS);
    const uint8_t changed = g_status_changed;
    const ctrl_motor_limit_event_t limit_event = (ctrl_motor_limit_event_t)g_limit_event;
    ctrl_motor_status_t status_snapshot;
    char first_line[MOTOR_STATUS_TEXT_BUFFER_SIZE];
    char second_line[MOTOR_STATUS_TEXT_BUFFER_SIZE];

    if (g_motor_initialized == 0U)
    {
        return;
    }

    if (periodic_due == 0U && changed == 0U && limit_event == CTRL_MOTOR_LIMIT_NONE)
    {
        return;
    }

    ctrl_motor_get_status(&status_snapshot);

    snprintf(first_line,
             sizeof(first_line),
             "Dir:%s P:%4d%%",
             ctrl_motor_direction_to_text(status_snapshot.motor_state.direction),
             (int)status_snapshot.applied_power_percent);

    snprintf(second_line,
             sizeof(second_line),
             "PWM:%3u T:%4d%%",
             status_snapshot.motor_state.pwm_value,
             (int)status_snapshot.target_power_percent);

    ctrl_stdio_lcd_print_two_lines(first_line, second_line);

    printf("[MOTOR] dir=%s target=%d%% applied=%d%% pwm=%u\n",
           ctrl_motor_direction_to_text(status_snapshot.motor_state.direction),
           (int)status_snapshot.target_power_percent,
           (int)status_snapshot.applied_power_percent,
           status_snapshot.motor_state.pwm_value);

#if (MOTOR_LIMIT_MESSAGE_ENABLED == 1)
    if (limit_event == CTRL_MOTOR_LIMIT_MAX)
    {
        printf("[MOTOR] limit reached: +100%%\n");
    }
    else if (limit_event == CTRL_MOTOR_LIMIT_MIN)
    {
        printf("[MOTOR] limit reached: -100%%\n");
    }
#endif

    g_limit_event = (uint8_t)CTRL_MOTOR_LIMIT_NONE;
    g_status_changed = 0U;
    last_report_time = now;
}

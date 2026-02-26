#include "ctrl_interfone.h"

static dd_led_t green_led;
static dd_led_t red_led;

void ctrl_interfone_init(void) {
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);

    dd_led_init(&green_led, LED_GREEN_PIN, digitalWrite);
    dd_led_init(&red_led, LED_RED_PIN, digitalWrite);

    ctrl_stdio_lcd_keypad_init();
}   


// helper for masked password input using keypad/LCD stream
static void read_password(char *buf, size_t max_len)
{
    size_t idx = 0;
    while (idx < max_len) {
        char c = getchar();          // blocking read from keypad/serial
        if (c == '#') {              // submit key
            break;
        }
        if (c == '*') {              // treat '*' as backspace
            if (idx > 0) {
                idx--;
                // erase star on display
                printf("\b \b");
            }
        } else {
            buf[idx++] = c;
            putchar('*');            // echo star
        }
    }
    buf[idx] = '\0';
}

void ctrl_interfone_update(void) {
   char input_data[INPUT_BUFFER_SIZE];

   // show prompt on first row and position cursor to second row
   printf("\fEnter password: \n");
   ctrl_stdio_set_cursor(0, 1);
   // read up to length of expected password; '#' submits early, '*' deletes
   read_password(input_data, strlen(USER_PASSWORD));

   if (strcmp(input_data, USER_PASSWORD) == 0) {
        // clear display then show result on top row
        printf("\fAccess granted!\n");
        dd_led_set_on(&green_led);
        dd_led_set_off(&red_led);
    } else {
        printf("\fAccess denied!\n");
        dd_led_set_off(&green_led);
        dd_led_set_on(&red_led);
    }

    uint32_t wait_time = WAIT_TIME_MS + millis(); // 5   seconds
    while (millis() < wait_time);

    dd_led_set_off(&green_led);
    dd_led_set_off(&red_led);
}
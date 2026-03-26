#include "ctrl_interfone.h"

static dd_led_t green_led;
static dd_led_t blue_led;

void ctrl_interfone_init(void) {
    dd_led_init(&green_led, LED_GREEN_PIN, DD_LED_OUTPUT, digitalWrite, pinMode);
    dd_led_init(&blue_led, LED_BLUE_PIN, DD_LED_OUTPUT, digitalWrite, pinMode);

    ctrl_stdio_lcd_keypad_init();
}   

void ctrl_interfone_update(void) {
   char input_data[INPUT_BUFFER_SIZE];

   printf("\fEnter password: \n");
   //TODO: Enhance input handling to support backspace and other editing features
    scanf("%4s", input_data);

    if (strcmp(input_data, USER_PASSWORD) == 0) {
        printf("Access granted!     \n");
        dd_led_set_on(&green_led);
        dd_led_set_off(&blue_led);
    } else {
        printf("Access denied!      \n");
        dd_led_set_on(&blue_led);
        dd_led_set_off(&green_led);
    }

    uint32_t wait_time = WAIT_TIME_MS + millis(); // 5   seconds
    while (millis() < wait_time);

    dd_led_set_off(&green_led);
    dd_led_set_off(&blue_led);
}
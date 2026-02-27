#ifndef CONFIGS_H
#define CONFIGS_H

#define BAUDRATE 115200

// Define constants for the system
#define LED_RED_PIN 14
#define LED_GREEN_PIN 15
#define LCD_COLS 16
#define LCD_ROWS 2
#define LCD_ADDR 0x27

#define INPUT_BUFFER_SIZE 100
#define USER_PASSWORD "1234"

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

#define WAIT_TIME_MS 3000

#define BUTTON_PIN 11   // btn1 -> mega:11 (green)
#define BUTTON_UP_PIN 10   // btn2 -> mega:10 (green)
#define BUTTON_DOWN_PIN 9    // btn3 -> mega:9  (green)
#define BUTTON_DEBOUNCE_DELAY 100 // ms

#define DEFAULT_BLINK_FREQUENCY 500 // ms
#define MIN_BLINK_FREQUENCY 100 // ms
#define MAX_BLINK_FREQUENCY 2000 // ms
#define BLINK_FREQUENCY_STEP 100 // ms

#define BUTTON_LED_TASK_OFFSET_MS       1
#define BUTTON_LED_TASK_RECURRANCE_MS   50

#define RED_LED_TASK_OFFSET_MS              5
#define RED_LED_TASK_RECURRANCE_MS          (MIN_BLINK_FREQUENCY / 2)

#define INC_DEC_LED_TASK_OFFSET_MS         2
#define INC_DEC_LED_TASK_RECURRANCE_MS     (BLINK_FREQUENCY_STEP / 2)

// #define BUTTON_LED_TASK_OFFSET_MS       1
// #define BUTTON_LED_TASK_RECURRANCE_MS   50

// #define LCD_TASK_OFFSET_MS              5
// #define LCD_TASK_RECURRANCE_MS          (MIN_BLINK_FREQUENCY / 2)



typedef enum {
    KEYPAD_PIN_1=23,
    KEYPAD_PIN_2=25,
    KEYPAD_PIN_3=27,  
    KEYPAD_PIN_4=29,
    KEYPAD_PIN_5=31,
    KEYPAD_PIN_6=33,
    KEYPAD_PIN_7=35,
    KEYPAD_PIN_8=37,     
}keypad_pin_t;

#endif // CONFIGS_H
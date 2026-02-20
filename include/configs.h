#ifndef CONFIGS_H
#define CONFIGS_H

#define BAUDRATE 115200

// Define constants for the system
#define LED_RED_PIN 14
#define LED_GREEN_PIN 15
#define LCD_COLS 16
#define LCD_ROWS 2

#define INPUT_BUFFER_SIZE 100
#define USER_PASSWORD "1234"

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

#define WAIT_TIME_MS 5000

typedef enum {
    KEYPAD_KEY_1=23,
    KEYPAD_KEY_2=25,
    KEYPAD_KEY_3=27,  
    KEYPAD_KEY_4=29,
    KEYPAD_KEY_5=31,
    KEYPAD_KEY_6=33,
    KEYPAD_KEY_7=35,
    KEYPAD_KEY_8=37,     
}keypad_pin_t;

// Row and column pin numbers for Keypad library
#define KEYPAD_PIN_1 23
#define KEYPAD_PIN_2 25
#define KEYPAD_PIN_3 27
#define KEYPAD_PIN_4 29
#define KEYPAD_PIN_5 31
#define KEYPAD_PIN_6 33
#define KEYPAD_PIN_7 35
#define KEYPAD_PIN_8 37

#endif // CONFIGS_H
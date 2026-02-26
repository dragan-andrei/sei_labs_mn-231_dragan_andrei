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

#define LCD_I2C_ADDRESS 0x27

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
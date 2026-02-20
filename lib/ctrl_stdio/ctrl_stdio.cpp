#include "ctrl_stdio.h"

// Two separate streams: one for Serial, one for LCD+Keypad
static FILE serial_stream = {0};
static FILE lcd_stream    = {0};

// в”Ђв”Ђ Keypad в”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђ
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte rowPins[KEYPAD_ROWS] = {KEYPAD_PIN_1, KEYPAD_PIN_2, KEYPAD_PIN_3, KEYPAD_PIN_4};
byte colPins[KEYPAD_COLS] = {KEYPAD_PIN_5, KEYPAD_PIN_6, KEYPAD_PIN_7, KEYPAD_PIN_8};

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

// в”Ђв”Ђ LCD в”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђв”Ђ
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

static uint8_t lcd_col = 0;
static uint8_t lcd_row = 0;

// Write a character to the LCD, handling newline / carriage-return
int ctrl_stdio_lcd_putchar(char ch, FILE* f)
{
    if (ch == '\r') {
        lcd_col = 0;
        lcd_row = 0;
        lcd.setCursor(lcd_col, lcd_row);
    } else if (ch == '\n') {
        lcd_col = 0;
        lcd_row = (lcd_row + 1) % LCD_ROWS;
        lcd.setCursor(lcd_col, lcd_row);
    } else {
        lcd.print(ch);
        if (++lcd_col >= LCD_COLS) {
            lcd_col = 0;
            lcd_row = (lcd_row + 1) % LCD_ROWS;
            lcd.setCursor(lcd_col, lcd_row);
        }
    }
    return 0;
}

// Read a character from the keypad (blocking)
int ctrl_stdio_keypad_getchar(FILE* f)
{
    char key = kpd.getKey();
    while (key == NO_KEY) {
        key = kpd.getKey();
    }
    return key;
}


// -- Serial -------------------------------------------------------------------
int ctrl_stdio_putchar(char ch, FILE* f)
{
    if (ch == '\n') {
        Serial.write('\r');
    }
    Serial.write(ch);
    return 0;
}

int ctrl_stdio_getchar(FILE* f)
{
    while (Serial.available() == 0);
    return Serial.read();
}

// -- Public init functions ----------------------------------------------------

// Redirect stdin/stdout/stderr to Serial UART
void ctrl_stdio_serial_init()
{
    Serial.begin(BAUDRATE);
    fdev_setup_stream(&serial_stream,
                      ctrl_stdio_putchar,
                      ctrl_stdio_getchar,
                      _FDEV_SETUP_RW);
    stdin = stdout = stderr = &serial_stream;
}

// Redirect stdin/stdout/stderr to LCD + Keypad
void ctrl_stdio_lcd_keypad_init()
{
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd_col = 0;
    lcd_row = 0;
    fdev_setup_stream(&lcd_stream,
                      ctrl_stdio_lcd_putchar,
                      ctrl_stdio_keypad_getchar,
                      _FDEV_SETUP_RW);
    stdin = stdout = stderr = &lcd_stream;
}
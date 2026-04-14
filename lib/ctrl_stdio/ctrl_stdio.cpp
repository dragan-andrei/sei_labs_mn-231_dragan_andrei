#include "ctrl_stdio.h"

// Two separate streams: one for Serial, one for LCD+Keypad
static FILE serial_stream = {0};
static FILE lcd_stream    = {0};
static uint8_t lcd_initialized = 0;

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte rowPins[KEYPAD_ROWS] = {KEYPAD_PIN_1, KEYPAD_PIN_2, KEYPAD_PIN_3, KEYPAD_PIN_4};
byte colPins[KEYPAD_COLS] = {KEYPAD_PIN_5, KEYPAD_PIN_6, KEYPAD_PIN_7, KEYPAD_PIN_8};

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

static uint8_t lcd_col = 0;
static uint8_t lcd_row = 0;

static void ctrl_stdio_lcd_ensure_initialized(void)
{
    if (lcd_initialized != 0U)
    {
        return;
    }

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd_col = LCD_HOME_COL;
    lcd_row = LCD_HOME_ROW;
    lcd.setCursor(LCD_HOME_COL, LCD_HOME_ROW);
    lcd_initialized = 1;
}

int ctrl_stdio_lcd_putchar(char ch, FILE* f)
{
    (void)f;
    ctrl_stdio_lcd_ensure_initialized();

    if (ch == '\f') {                 
        lcd.clear();
        lcd_col = LCD_HOME_COL;
        lcd_row = LCD_HOME_ROW;
        lcd.setCursor(LCD_HOME_COL, LCD_HOME_ROW);
    }
    else if (ch == '\r') {
        lcd_col = LCD_HOME_COL;
        lcd_row = LCD_HOME_ROW;
        lcd.setCursor(lcd_col, lcd_row);
    }
    else if (ch == '\n') {
        lcd_col = LCD_HOME_COL;
        lcd_row = (lcd_row + 1) % LCD_ROWS;
        lcd.setCursor(lcd_col, lcd_row);
    }
    else {
        lcd.print(ch);
        if (++lcd_col >= LCD_COLS) {
            lcd_col = LCD_HOME_COL;
            lcd_row = (lcd_row + 1) % LCD_ROWS;
            lcd.setCursor(lcd_col, lcd_row);
        }
    }
    return 0;
}

// Read a character from the keypad (blocking)
int ctrl_stdio_keypad_getchar(FILE* f)
{
    (void)f;
    char key = NO_KEY;

    while (ctrl_stdio_keypad_read_char(&key) == false) {
    }

    return key;
}

bool ctrl_stdio_keypad_read_char(char *key)
{
    const char current_key = kpd.getKey();

    if (key == NULL || current_key == NO_KEY)
    {
        return false;
    }

    *key = current_key;
    return true;
}


// -- Serial -------------------------------------------------------------------
int ctrl_stdio_putchar(char ch, FILE* f)
{
    (void)f;
    if (ch == '\n') {
        Serial.write('\r');
    }
    Serial.write(ch);
    return 0;
}

int ctrl_stdio_getchar(FILE* f)
{
    (void)f;
    char character = '\0';

    while (ctrl_stdio_serial_read_char(&character) == false)
    {
    }

    return character;
}

bool ctrl_stdio_serial_read_char(char *character)
{
    if (character == NULL || Serial.available() == 0)
    {
        return false;
    }

    *character = (char)Serial.read();
    return true;
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

void ctrl_stdio_lcd_init(void)
{
    ctrl_stdio_lcd_ensure_initialized();
}

void ctrl_stdio_lcd_print_two_lines(const char *first_line, const char *second_line)
{
    ctrl_stdio_lcd_ensure_initialized();

    lcd.clear();

    lcd.setCursor(LCD_HOME_COL, LCD_HOME_ROW);
    if (first_line != NULL)
    {
        for (uint8_t i = 0; i < LCD_COLS && first_line[i] != '\0'; i++)
        {
            lcd.print(first_line[i]);
        }
    }

    lcd.setCursor(LCD_HOME_COL, LCD_SECOND_ROW);
    if (second_line != NULL)
    {
        for (uint8_t i = 0; i < LCD_COLS && second_line[i] != '\0'; i++)
        {
            lcd.print(second_line[i]);
        }
    }
}

// Redirect stdin/stdout/stderr to LCD + Keypad
void ctrl_stdio_lcd_keypad_init()
{
    ctrl_stdio_lcd_ensure_initialized();
    lcd.clear();
    lcd_col = LCD_HOME_COL;
    lcd_row = LCD_HOME_ROW;
    fdev_setup_stream(&lcd_stream,
                      ctrl_stdio_lcd_putchar,
                      ctrl_stdio_keypad_getchar,
                      _FDEV_SETUP_RW);
    stdin = stdout = stderr = &lcd_stream;
}
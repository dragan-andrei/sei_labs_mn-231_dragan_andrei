#include "ctrl_stdio.h"
#include <stdarg.h>

// Global output mode
int g_output_mode = OUTPUT_SERIAL;

// Keypad configuration
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte rowPins[KEYPAD_ROWS] = {KEYPAD_PIN_1, KEYPAD_PIN_2, KEYPAD_PIN_3, KEYPAD_PIN_4};
byte colPins[KEYPAD_COLS] = {KEYPAD_PIN_5, KEYPAD_PIN_6, KEYPAD_PIN_7, KEYPAD_PIN_8};

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

// LCD setup
LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS);

static uint8_t lcd_col = 0;
static uint8_t lcd_row = 0;
static bool lcd_initialized = false;

// ============================================================================
// LCD OUTPUT FUNCTIONS
// ============================================================================

void ctrl_stdio_lcd_putchar(char ch)
{
    if (!lcd_initialized) return;
    
    if (ch == '\f') {                 
        lcd.clear();
        lcd_col = 0;
        lcd_row = 0;
        lcd.setCursor(0, 0);
    }
    else if (ch == '\r') {
        lcd_col = 0;
        lcd.setCursor(lcd_col, lcd_row);
    }
    else if (ch == '\n') {
        lcd_col = 0;
        lcd_row = (lcd_row + 1) % LCD_ROWS;
        lcd.setCursor(lcd_col, lcd_row);
    }
    else if (ch == '\b') {
        if (lcd_col > 0) {
            lcd_col--;
        } else if (lcd_row > 0) {
            lcd_row--;
            lcd_col = LCD_COLS - 1;
        }
        lcd.setCursor(lcd_col, lcd_row);
        lcd.print(' ');
        lcd.setCursor(lcd_col, lcd_row);
    } 
    else if (ch >= 32 && ch < 127) {  // Printable ASCII only
        lcd.print(ch);
        if (++lcd_col >= LCD_COLS) {
            lcd_col = 0;
            lcd_row = (lcd_row + 1) % LCD_ROWS;
            lcd.setCursor(lcd_col, lcd_row);
        }
    }
}

void ctrl_stdio_lcd_print(const char *str)
{
    if (!str) return;
    for (int i = 0; str[i]; i++) {
        ctrl_stdio_lcd_putchar(str[i]);
    }
}

void ctrl_stdio_lcd_printf(const char *fmt, ...)
{
    char buf[64];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    ctrl_stdio_lcd_print(buf);
}

void ctrl_stdio_lcd_clear(void)
{
    ctrl_stdio_lcd_putchar('\f');
}

// ============================================================================
// PUBLIC INIT FUNCTIONS
// ============================================================================

// Initialize Serial (printf goes here by default on ESP32)
void ctrl_stdio_serial_init()
{
    Serial.begin(BAUDRATE);
    g_output_mode = OUTPUT_SERIAL;
    Serial.printf("\n\n=== SERIAL INITIALIZED ===\n");
    Serial.printf("Output: Serial/USB\n\n");
}

// Initialize LCD + Keypad
void ctrl_stdio_lcd_keypad_init()
{
    // Also initialize Serial for debugging/fallback
    Serial.begin(BAUDRATE);
    
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd_col = 0;
    lcd_row = 0;
    lcd_initialized = true;
    
    g_output_mode = OUTPUT_LCD;
    
    // Show init message
    lcd.print("System Init...");
    delay(1000);
    lcd.clear();
    lcd_col = 0;
    lcd_row = 0;
}

// position cursor anywhere on the LCD
void ctrl_stdio_set_cursor(uint8_t col, uint8_t row)
{
    if (col >= LCD_COLS) col = LCD_COLS - 1;
    if (row >= LCD_ROWS) row = LCD_ROWS - 1;
    lcd_col = col;
    lcd_row = row;
    lcd.setCursor(col, row);
}

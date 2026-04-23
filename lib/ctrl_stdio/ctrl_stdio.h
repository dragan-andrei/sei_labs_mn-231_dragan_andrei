#ifndef ctrl_stdio_h
#define ctrl_stdio_h

#include <Arduino.h>
#include <stdio.h>

#define BAUDRATE 115200

#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#include "configs.h"

// Global output mode
extern int g_output_mode;
#define OUTPUT_SERIAL 0
#define OUTPUT_LCD 1

void ctrl_stdio_serial_init(void);
void ctrl_stdio_lcd_keypad_init(void);

// move LCD cursor to given column/row (0-based)
void ctrl_stdio_set_cursor(uint8_t col, uint8_t row);

// LCD output functions
void ctrl_stdio_lcd_print(const char *str);
void ctrl_stdio_lcd_printf(const char *fmt, ...);
void ctrl_stdio_lcd_clear(void);

#endif


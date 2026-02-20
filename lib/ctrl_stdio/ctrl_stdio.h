#ifndef ctrl_stdio_h
#define ctrl_stdio_h

#include <Arduino.h>
#include <stdio.h>

#define BAUDRATE 115200

#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#include "configs.h"

void ctrl_stdio_serial_init(void);
void ctrl_stdio_lcd_keypad_init(void);


#endif


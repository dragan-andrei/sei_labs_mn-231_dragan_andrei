#ifndef CTRL_STDIO_H
#define CTRL_STDIO_H

#include <Arduino.h>

// Inițializează comunicarea serială
void ctrl_stdio_init(unsigned long baudrate);
// Formatează și afișează un număr extras din coadă
void ctrl_stdio_print_data(uint8_t data);
// Trece pe un rând nou
void ctrl_stdio_print_newline();
// Afișează un șir de caractere (text)
void ctrl_stdio_print_text(const char* text);
// Alias de compatibilitate pentru afișare text
void ctrl_stdio_print(const char* text);
// Funcție generică tip printf
void ctrl_stdio_printf(const char *fmt, ...);

#endif // CTRL_STDIO_H
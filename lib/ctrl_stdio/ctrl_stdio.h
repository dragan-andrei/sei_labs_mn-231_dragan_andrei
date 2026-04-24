#ifndef CTRL_STDIO_H
#define CTRL_STDIO_H

#include <Arduino.h>

// Inițializează comunicarea serială
void ctrl_stdio_init(unsigned long baudrate);

// Afișează un șir de caractere (text brut) cu normalizare CR+LF
void ctrl_stdio_print_text(const char* text);

// Alias de compatibilitate
void ctrl_stdio_print(const char* text);

// Trece pe un rând nou
void ctrl_stdio_print_newline();

// Funcție generică tip printf (buffer intern 160 de octeți)
void ctrl_stdio_printf(const char* fmt, ...);

#endif // CTRL_STDIO_H

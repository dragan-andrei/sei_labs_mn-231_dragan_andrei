#include "ctrl_stdio.h"

void ctrl_stdio_init(unsigned long baudrate) {
    Serial.begin(baudrate);
}

void ctrl_stdio_print_data(uint8_t data) {
    Serial.print("Val: ");
    Serial.print(data);
    Serial.print(" | ");
}

void ctrl_stdio_print_newline() {
    Serial.println();
}

void ctrl_stdio_print_text(const char* text) {
    if (text == NULL) {
        return;
    }

    char previous = '\0';
    for (const char* p = text; *p != '\0'; ++p) {
        if (*p == '\n' && previous != '\r') {
            Serial.write('\r');
        }
        Serial.write(*p);
        previous = *p;
    }
}

void ctrl_stdio_print(const char* text) {
    ctrl_stdio_print_text(text);
}

void ctrl_stdio_printf(const char *fmt, ...) {
    char buf[128];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    ctrl_stdio_print_text(buf);
}


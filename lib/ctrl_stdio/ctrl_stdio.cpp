#include "ctrl_stdio.h"

#include <stdarg.h>
#include <stdio.h>

static constexpr size_t CTRL_STDIO_BUFFER_SIZE = 160;

void ctrl_stdio_init(unsigned long baudrate) {
    Serial.begin(baudrate);
}

void ctrl_stdio_print_text(const char* text) {
    if (text == nullptr) {
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

void ctrl_stdio_print_newline() {
    Serial.write('\r');
    Serial.write('\n');
}

void ctrl_stdio_printf(const char* fmt, ...) {
    if (fmt == nullptr) {
        return;
    }

    char buffer[CTRL_STDIO_BUFFER_SIZE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    ctrl_stdio_print_text(buffer);
}

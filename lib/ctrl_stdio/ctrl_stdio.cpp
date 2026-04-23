#include "ctrl_stdio.h"

// Stream-ul virtual pentru serial
static FILE serial_stream = {0};

// Functie de output pentru printf
static int ctrl_stdio_putchar(char ch, FILE* f) {
    (void)f;
    if (ch == '\n') {
        Serial.write('\r');
    }
    Serial.write(ch);
    return 0;
}

// Functie de input
static int ctrl_stdio_getchar(FILE* f) {
    (void)f;
    while (Serial.available() == 0);
    return Serial.read();
}

void ctrl_stdio_serial_init(void) {
    Serial.begin(BAUDRATE);
    while (!Serial) {
        ; // Asteptam conectarea portului serial
    }

    // Redirectionarea stdout, stdin, stderr pentru a folosi printf / scanf
    fdev_setup_stream(&serial_stream, ctrl_stdio_putchar, ctrl_stdio_getchar, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &serial_stream;
}
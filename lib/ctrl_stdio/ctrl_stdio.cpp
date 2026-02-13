#include "ctrl_stdio.h"

static FILE stream = {0};

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

void ctrl_stdio_init() 
{
  Serial.begin(BAUDRATE);
  fdev_setup_stream(&stream, 
                    ctrl_stdio_putchar, 
                    ctrl_stdio_getchar,
                    _FDEV_SETUP_RW
                );
    stdin = stdout = stderr = &stream;
}
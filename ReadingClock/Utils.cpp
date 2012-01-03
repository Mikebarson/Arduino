#include <Arduino.h>
#include "utils.h"

#define BUF_LEN 25

static const char * formatString_Core(const char *format, va_list args)
{
  static char buf[BUF_LEN];

  vsnprintf(buf, sizeof(buf), format, args);

  return buf;
}

const char * formatString_P(const char *formatP, ...)
{
  static char format[BUF_LEN];

  int i = 0;
  char ch;
  do
  {
    ch = pgm_read_byte(formatP++);
    format[i++] = ch;
  } while ((i < BUF_LEN) && (ch != 0));

  va_list args;
  va_start(args, formatP);
  const char * result = formatString_Core(format, args);
  va_end(args);

  return result;
}

const char * formatString(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  const char * result = formatString_Core(format, args);
  va_end(args);
  
  return result;
}

const char * toString(int n)
{
  return formatString_P(PSTR("%0.2d"), n);
}

int freeRam()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


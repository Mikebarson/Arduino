#include <Arduino.h>
#include "utils.h"

char * formatString(char *format, ...)
{
  static char buf[50];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  
  return buf;
}

char * toString(int n)
{
  return formatString("%0.2d", n);
}



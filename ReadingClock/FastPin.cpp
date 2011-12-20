#include <Arduino.h>
#include "FastPin.h"

FastPin::FastPin(int pin)
  : port(GetPort(pin)), mask(GetMask(pin))
{
}

void FastPin::DigitalWrite(int state)
{
  if (state)
  {
    port |= mask;
  }
  else
  {
    port &= ~mask;
  }
}


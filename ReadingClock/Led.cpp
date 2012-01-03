#include <Arduino.h>
#include "Led.h"

Led::Led(int pin)
  : _pin(pin)
{
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

void Led::Enable()
{
  digitalWrite(_pin, HIGH);
}

void Led::Disable()
{
  digitalWrite(_pin, LOW);
}

void Led::SetState(bool on)
{
  digitalWrite(_pin, on ? HIGH : LOW);
}

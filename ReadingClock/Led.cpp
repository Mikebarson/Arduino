#include <Arduino.h>
#include "Led.h"

Led::Led(int pin)
  : _pin(pin)
{
  pinMode(pin, OUTPUT);
  _pin.DigitalWrite(LOW);
}

void Led::Enable()
{
  _pin.DigitalWrite(HIGH);
}

void Led::Disable()
{
  _pin.DigitalWrite(LOW);
}

void Led::SetState(bool on)
{
  _pin.DigitalWrite(on ? HIGH : LOW);
}

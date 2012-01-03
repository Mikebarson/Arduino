#include <Arduino.h>
#include "Button.h"

Button::Button(int pin)
  : pin(pin), lastState(false)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, LOW);
}

bool Button::WasPressed()
{
  bool state = digitalRead(pin) == 1;
  bool wasPressed = !lastState && state;
  lastState = state;
  return wasPressed;
}


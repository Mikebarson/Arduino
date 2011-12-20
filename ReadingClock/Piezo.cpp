#include <Arduino.h>
#include "Piezo.h"

Piezo::Piezo(int frequencyPin, int onOffPin, int frequency)
  : _onOffPin(onOffPin)
{
  pinMode(onOffPin, OUTPUT);
  _onOffPin.DigitalWrite(LOW);
  
  _tone.begin(frequencyPin);
  _tone.play(frequency);
}

void Piezo::SetState(bool on)
{
  _onOffPin.DigitalWrite(on ? HIGH : LOW);
}

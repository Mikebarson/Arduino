#include <Arduino.h>
#include "Piezo.h"

Piezo::Piezo(int frequencyPin, int onOffPin, int frequency)
  : _onOffPin(onOffPin), _frequency(frequency)
{
  pinMode(onOffPin, OUTPUT);
  _onOffPin.DigitalWrite(LOW);
  
  _tone.begin(frequencyPin);
}

void Piezo::SetState(bool on)
{
  if (on)
  {
    _tone.play(_frequency);
  }
  else
  {
    _tone.stop();
  }

  _onOffPin.DigitalWrite(on ? HIGH : LOW);
}

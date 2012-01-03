#include "Piezo.h"

Piezo::Piezo(int frequencyPin, int onOffPin, int frequency)
  : _frequencyPin(frequencyPin), _onOffPin(onOffPin), _frequency(frequency)
{
  pinMode(_onOffPin, OUTPUT);
  digitalWrite(_onOffPin, LOW);
  
  _tone.begin(_frequencyPin);
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

  digitalWrite(_onOffPin, on ? HIGH : LOW);
}


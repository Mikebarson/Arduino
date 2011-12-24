#include <Arduino.h>
#include <Tone.h>
#include "FastPin.h"

class Piezo
{
  public:
    Piezo(int frequencyPin, int onOffPin, int frequency);
    
    void SetState(bool on);
    
  private:
    Tone _tone;
    int _frequency;
    FastPin _onOffPin;
};

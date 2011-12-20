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
    FastPin _onOffPin;
};

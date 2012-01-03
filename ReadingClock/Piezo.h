#ifndef _piezo_h
#define _piezo_h

#include <Arduino.h>
#include <Tone.h>

class Piezo
{
  public:
    Piezo(int frequencyPin, int onOffPin, int frequency);
    
    void SetState(bool on);
    
  private:
    Tone _tone;
    int _frequency;
    int _frequencyPin;
    int _onOffPin;
};

#endif // _piezo_h


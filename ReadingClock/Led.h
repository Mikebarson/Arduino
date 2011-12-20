#include "FastPin.h"

class Led
{
  public:
    Led(int pin);

    void Enable();
    void Disable();    
    void SetState(bool on);
    
  private:
    FastPin _pin;
};


#ifndef _led_h
#define _led_h

class Led
{
  public:
    Led(int pin);

    void Enable();
    void Disable();    
    void SetState(bool on);
    
  private:
    int _pin;
};

#endif // _led_h


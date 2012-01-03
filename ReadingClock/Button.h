#ifndef _button_h
#define _button_h

class Button
{
  public:
    Button(int pin);

    bool WasPressed();
    
  private:
    int pin;
    volatile bool lastState;
};

#endif // _button_h


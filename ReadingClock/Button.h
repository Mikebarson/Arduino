class Button
{
  public:
    Button(int pin);

    bool WasPressed();
    
  private:
    int pin;
    volatile bool lastState;
};


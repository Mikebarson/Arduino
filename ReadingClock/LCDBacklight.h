class LCDBacklight
{
  private:
    int redPin, greenPin, bluePin;
    byte red, green, blue;
    bool enabled;
  
  public:
    LCDBacklight(int redPin, int greenPin, int bluePin);

    void SetColor(byte red, byte green, byte blue);
    void Enable();
    void Disable();
    
  private:
    void WriteToDevice(byte red, byte green, byte blue);
};


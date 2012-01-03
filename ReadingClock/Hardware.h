#ifndef _hardware_h
#define _hardware_h

#include <Tone.h>

class Pins
{
  public:
    enum Values
    {
      lcdBacklightRed = 9,
      lcdBacklightGreen = 6,
      lcdBacklightBlue = 5,
      lcdSID = 13,
      lcdSCLK = 12,
      lcdA0 = 11,
      lcdRST = 10,
      lcdCS = 8,
      RTC_SQW = 7,
      RTC_SDA = A4,
      RTC_SCL = A5,
      encoderA = A0,
      encoderB = A1,
      encoderButton = A2,
      alarmButton = A3,
      alarmLed = 2,
      piezoFrequency = 3,
      piezoEnable = 4,
    };
};

class Button
{
  public:
    Button(int pin);

    bool WasPressed();
    
  private:
    int pin;
    volatile bool lastState;
};

class Encoder
{
  public:
    Encoder(int pinA, int pinB);
    
    void Init();
    int ReadDelta();
    
  private:
    int pinA, pinB;
    volatile byte state;
};

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

#endif // _hardware_h


#include "Arduino.h"
#include "LCDBacklight.h"

LCDBacklight::LCDBacklight(int redPin, int greenPin, int bluePin)
  : redPin(redPin), greenPin(greenPin), bluePin(bluePin),
    enabled(false)
{
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}
  
void LCDBacklight::SetColor(byte red, byte green, byte blue)
{
  this->red = red;
  this->green = green;
  this->blue = blue;
  
  if (enabled)
  {
    WriteToDevice(red, green, blue);
  }
}

void LCDBacklight::Enable()
{
  enabled = true;
  WriteToDevice(red, green, blue);
}

void LCDBacklight::Disable()
{
  enabled = false;
  WriteToDevice(0, 0, 0);
}

void LCDBacklight::WriteToDevice(byte red, byte green, byte blue)
{
    analogWrite(redPin, 255 - red);
    analogWrite(greenPin, 255 - green);
    analogWrite(bluePin, 255 - blue);
}


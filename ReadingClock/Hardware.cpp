#include <Arduino.h>
#include "Hardware.h"

Button::Button(int pin)
  : pin(pin), lastState(false)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, LOW);
}

bool Button::WasPressed()
{
  bool state = digitalRead(pin) == 1;
  bool wasPressed = !lastState && state;
  lastState = state;
  return wasPressed;
}


Encoder::Encoder(int pinA, int pinB)
  : pinA(pinA), pinB(pinB), state(0)
{
  pinMode(pinA, INPUT);
  digitalWrite(pinA, LOW);
  pinMode(pinB, INPUT);
  digitalWrite(pinB, LOW);  
}

void Encoder::Init()
{
  ReadDelta();
}

int Encoder::ReadDelta()
{
  static const int8_t enc_states[] =
  {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
  };

  int ab =
    digitalRead(pinA) << 1 |
    digitalRead(pinB);

  state = ((state << 2) | ab) & 0x0f;
  return enc_states[state];
}


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

Led::Led(int pin)
  : _pin(pin)
{
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

void Led::Enable()
{
  digitalWrite(_pin, HIGH);
}

void Led::Disable()
{
  digitalWrite(_pin, LOW);
}

void Led::SetState(bool on)
{
  digitalWrite(_pin, on ? HIGH : LOW);
}

Piezo::Piezo(int frequencyPin, int onOffPin, int frequency)
  : _frequencyPin(frequencyPin), _onOffPin(onOffPin), _frequency(frequency)
{
  pinMode(_onOffPin, OUTPUT);
  digitalWrite(_onOffPin, LOW);
  
  _tone.begin(_frequencyPin);
}

void Piezo::SetState(bool on)
{
  if (on)
  {
    _tone.play(_frequency);
  }
  else
  {
    _tone.stop();
  }

  digitalWrite(_onOffPin, on ? HIGH : LOW);
}


#include <Arduino.h>
#include "Encoder.h"

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


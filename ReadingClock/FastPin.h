#ifndef _fastpin_h
#define _fastpin_h

#define GetPort(pin) \
  (pin >= 0 && pin <= 7)  \
    ? PORTD  \
    : (pin >= 8 && pin <= 13) \
      ? PORTB  \
      : (pin >= A0 && pin <= A7)  \
        ? PORTC  \
        : PORTD
        
#define GetBit(pin) \
  (pin == 0 || pin == 8 || pin == A0)  \
    ? 0  \
    : (pin == 1 || pin == 9 || pin == A1) \
      ? 1  \
      : (pin == 2 || pin == 10 || pin == A2) \
        ? 2 \
        : (pin == 3 || pin == 11 || pin == A3) \
          ? 3 \
          : (pin == 4 || pin == 12 || pin == A4) \
            ? 4 \
            : (pin == 5 || pin == 13 || pin == A5) \
              ? 5 \
              : pin

#define GetMask(pin) 1 << (GetBit(pin))

class FastPin
{
  public:
    FastPin(int pin);

    void DigitalWrite(int state);

  private:
    volatile byte &port;
    byte mask;
};

#endif _fastpin_h

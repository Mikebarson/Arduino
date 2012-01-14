#ifndef _pins_h
#define _pins_h

struct Pins
{
  enum Values
  {
    SD_ChipSelect = 10,
    SD_DataIn = 11,
    SD_DataOut = 12,
    SD_Clock = 13,
    Accel_X = A0,
    Accel_Y = A2,
    Accel_Z = A1,
    Aux_Temperature = A3,
    Barometer_SDA = A4,
    Barometer_SCL = A5,
    Barometer_XCLR = 4,
    LED_Ground = 7,
    LED_1 = 6,
    LED_2 = 5,
    LED_3 = 3,
  };
};

#endif // _pins_h


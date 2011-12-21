#ifndef _pins_h
#define _pins_h

/* Wire colors
BacklightBlue - Blue
BacklightGreen - Green
BacklightRed - Red
lcdSID - Orange
lcdSCLK - Yellow
lcdA0 - Brown
lcdRST - Gray
SquareWave - Purple
AlarmFlash - White
PiezoFreq - Yellow
PiezoEnable - Green
*/

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

#endif _pins_h

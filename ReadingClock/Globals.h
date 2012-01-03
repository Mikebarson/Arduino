#ifndef _globals_h
#define _globals_h

#include <Arduino.h>
#include <GLCD_ST7565.h>
#include <RTClib.h>
#include "Hardware.h"
#include "Timer.h"
#include "Settings.h"

class States
{
  public:
    enum Values
    {
      idle,
      timerRunning,
      timerRunningAndExpired,
      timerPaused,
      timerStopped,
      menu,
    };
};

extern GLCD_ST7565 glcd;
extern RTC_DS1307 RTC;

extern LCDBacklight lcdBacklight;
extern Encoder encoder;
extern Piezo piezo;
extern Timer timer;

extern Button encoderButton;
extern Button alarmButton;
extern Led alarmLed;

extern volatile int encoderCountRaw;
extern volatile int encoderButtonPressCount;
extern volatile int alarmButtonPressCount;
extern volatile bool turnLightsOn;
extern volatile int squareCount;

extern int currentState;

extern int beforeMenuState;

extern Settings settings;

#endif // _globals_h


#include "globals.h"

GLCD_ST7565 glcd;
RTC_DS1307 RTC;

LCDBacklight lcdBacklight(Pins::lcdBacklightRed, Pins::lcdBacklightGreen, Pins::lcdBacklightBlue);
Encoder encoder(Pins::encoderA, Pins::encoderB);
Piezo piezo(Pins::piezoFrequency, Pins::piezoEnable, 4000);
Timer timer;

Button encoderButton(Pins::encoderButton);
Button alarmButton(Pins::alarmButton);
Led alarmLed(Pins::alarmLed);

volatile int encoderCountRaw = 0;
volatile int encoderButtonPressCount = 0;
volatile int alarmButtonPressCount = 0;
volatile bool turnLightsOn = true;
volatile int squareCount;

int currentState = States::idle;

int beforeMenuState;

Settings settings;


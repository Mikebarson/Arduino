#include <Arduino.h>
#include <Wire.h>

#include <GLCD_ST7565.h>
#include <JeeLib.h>
#include <avr/pgmspace.h>
#include <utility/font_clR6x8.h>
#include <utility/font_luBS18.h>

#include <RTClib.h>

#include <EEPROM.h>
#include <Tone.h>

#include "Pins.h"
#include "LCDBacklight.h"
#include "EEPROMAddresses.h"
#include "States.h"
#include "Timer.h"
#include "Alarm.h"
#include "Encoder.h"
#include "Button.h"
#include "Led.h"
#include "Piezo.h"

static const long lightsOffDelayMillis = 5000;
static const int defaultContrast = 0x15;

GLCD_ST7565 glcd;
RTC_DS1307 RTC;

LCDBacklight lcdBacklight(Pins::lcdBacklightRed, Pins::lcdBacklightGreen, Pins::lcdBacklightBlue);
Encoder encoder(Pins::encoderA, Pins::encoderB);
Piezo piezo(Pins::piezoFrequency, Pins::piezoEnable, 4000);
Timer timer;
Alarm alarm(&UpdateAlarmOutputs);
Button encoderButton(Pins::encoderButton);
Button alarmButton(Pins::alarmButton);
Led alarmLed(Pins::alarmLed);

volatile int encoderCountRaw = 0;
volatile int encoderButtonPressCount = 0;
volatile int alarmButtonPressCount = 0;
volatile bool turnLightsOn = true;

int contrast = defaultContrast;
int currentState = States::idle;

volatile int squareCount;


void setup()
{
  Wire.begin();
  
  lcdBacklight.SetColor(255, 255, 255);
  
  configureLCD();
  configureInterrupts();
  configureRTC();
  
  timer.SetTimespan(5);

  encoder.Init();
  getTimeDeltaMillis();
}

void loop()
{
  int timeDeltaMillis = getTimeDeltaMillis();

  adjustLights();

  timer.Update(timeDeltaMillis);
  alarm.Update(timeDeltaMillis);
  updateCurrentState();
  
  glcd.clear();
  glcd.drawString(0, 0, toString(encoderCountRaw));
  glcd.drawString(0, 10, toString(encoderButtonPressCount));
  glcd.drawString(0, 20, toString(alarmButtonPressCount));
  glcd.drawString(0, 30, toString(timeDeltaMillis));
  glcd.drawString(0, 40, toString(squareCount));

  glcd.drawString(0, 50, "Elapsed Time: ");
  glcd.drawString(84, 50, toString(timer.GetElapsedSeconds()));
  glcd.refresh();
}

void configureInterrupts()
{
  // Pin change interrupt control register - enables interrupt vectors
  // Bit 2 = enable PC vector 2 (PCINT23..16)
  // Bit 1 = enable PC vector 1 (PCINT14..8)
  // Bit 0 = enable PC vector 0 (PCINT7..0)
  PCICR |= 1 << PCIE1 | 1 << PCIE2;
  
  PCMSK2 |=
    1 << PCINT23;
  
  // Pin change mask registers decide which pins are enabled as triggers
  PCMSK1 |=
    1 << PCINT8 |
    1 << PCINT9 |
    1 << PCINT10 |
    1 << PCINT11;
  
  // enable interrupts
  interrupts();
}

void updateCurrentState()
{
  int encoderDelta = getEncoderDelta();
  int alarmButtonDelta = getAlarmButtonDelta();
  int encoderButtonDelta = getEncoderButtonDelta();

  if (encoderButtonDelta > 0)
  {
    encoderCountRaw = 0;
  }
  
  switch (currentState)
  {
    case States::idle:
      if (alarmButtonDelta > 0)
      {
        controlRTCSquareWave(true);
        timer.Reset();
        timer.Start();
        currentState = States::timerRunning;
      }
      break;
      
    case States::timerRunning:
      {
        if (timer.IsExpired())
        {
          currentState = States::timerRunningAndExpired;
          alarm.TurnOn();
        }
        
        if (alarmButtonDelta > 0)
        {
          currentState = States::timerPaused;
          timer.Pause();
          controlRTCSquareWave(false);
        }
      }
      break;
      
    case States::timerRunningAndExpired:
      {
        if (alarmButtonDelta > 0)
        {
          currentState = States::idle;
          timer.Pause();
          alarm.TurnOff();
          controlRTCSquareWave(false);
        }
      }
      break;
      
    case States::timerPaused:
      if (alarmButtonDelta > 0)
      {
        currentState = States::timerRunning;
        timer.Start();
        controlRTCSquareWave(true);
      }
      break;
  }
}

ISR(WDT_vect)
{
  Sleepy::watchdogEvent();
}

ISR(PCINT2_vect)
{
  ++squareCount;
}

ISR(PCINT1_vect)
{
  turnLightsOn = true;
  
  encoderCountRaw += encoder.ReadDelta();
  
  encoderButtonPressCount += encoderButton.WasPressed() ? 1 : 0;
  alarmButtonPressCount += alarmButton.WasPressed() ? 1 : 0;
}

void configureLCD()
{
  /*
  contrast = EEPROM.read(EEPROMAddresses::lcdContrast);
  if (contrast == 0)
*/  {
    contrast = defaultContrast;
  }

  glcd.begin(contrast);
  glcd.setFont(font_clR6x8);
  
  delay(500);
}

void configureRTC()
{
  RTC.begin();
  controlRTCSquareWave(false);
}

void controlRTCSquareWave(bool enable)
{
  RTC.controlSquareWaveOutput(enable, RTC_DS1307::OneHz, false);
}

char * toString(int n)
{
  static char buf[24];

  snprintf(buf, 24, "%0.2d", n);
  return buf;
}

int getEncoderDelta()
{
  static int oldEncoderCountRaw = 0;
  
  int delta = (encoderCountRaw - oldEncoderCountRaw) / 4;
  if (abs(delta) >= 1)
  {
    oldEncoderCountRaw = encoderCountRaw;
  }
  
  return delta;
}

int getEncoderButtonDelta()
{
  static int oldEncoderButtonPressCount = 0;
  int delta = encoderButtonPressCount - oldEncoderButtonPressCount;
  oldEncoderButtonPressCount = encoderButtonPressCount;
  return delta;
}

int getAlarmButtonDelta()
{
  static int oldAlarmButtonPressCount = 0;
  int delta = alarmButtonPressCount - oldAlarmButtonPressCount;
  oldAlarmButtonPressCount = alarmButtonPressCount;
  return delta;
}

long getTimeDeltaMillis()
{
  static long oldMillis = 0;
  
  long nowMillis = millis();
  long deltaMillis = nowMillis - oldMillis;
  oldMillis = nowMillis;
  
  return deltaMillis;
}

void adjustLights()
{
  static long lastLightsOnMillis = 0;
  long currentMillis = millis();
  
  if (turnLightsOn)
  {
    lastLightsOnMillis = currentMillis;
    turnLightsOn = false;
    lcdBacklight.Enable();

    if (!alarm.IsOn())
    {
      alarmLed.Enable();
    }
    return;
  }
  
  bool turnLightsOff = (currentMillis - lastLightsOnMillis) > lightsOffDelayMillis;
  if (turnLightsOff)
  {
    lcdBacklight.Disable();

    if (!alarm.IsOn())
    {
      alarmLed.Disable();
    }
  }
}

void UpdateAlarmOutputs(bool alarmSignalOn)
{
  alarmLed.SetState(alarmSignalOn);
  piezo.SetState(alarmSignalOn);
}


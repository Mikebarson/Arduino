#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <Tone.h>
#include <EEPROM.h>

#include <JeeLib.h>
#include <GLCD_ST7565.h>
#include <avr/pgmspace.h>
#include <utility/font_clR6x8.h>
#include <utility/font_luBS18.h>

#include "globals.h"
#include "States.h"
#include "Alarm.h"
#include "menu.h"
#include "utils.h"

Alarm alarm(&UpdateAlarmOutputs);

void setup()
{
  Serial.begin(57600);
  
  settings.readAllSettings();
  
  configureLCD();
  configureInterrupts();
  configureRTC();

  timer.SetTimespan(settings.timerMinutes * 60);

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
    
  if (currentState == States::menu)
  {
    DrawMenu();
  }
  else
  {
    DrawHomeScreen();
  }

  glcd.refresh();
}

void DrawDebuggingScreen(int timeDeltaMillis)
{
  glcd.drawString(0, 0, formatString_P(PSTR("Encoder: %0.2d"), encoderCountRaw));
  glcd.drawString(0, 10, formatString_P(PSTR("Encoder Button: %0.2d"), encoderButtonPressCount));
  glcd.drawString(0, 20, formatString_P(PSTR("Alarm Button: %0.2d"), alarmButtonPressCount));
  glcd.drawString(0, 30, formatString_P(PSTR("Time Delta:"), timeDeltaMillis));
  glcd.drawString(0, 40, formatString_P(PSTR("Free RAM: %d"), freeRam()));
  glcd.drawString(0, 50, formatString_P(PSTR("Elapsed Time: %d"), timer.GetElapsedSeconds()));
}

void DrawHomeScreen()
{
  const char * line;
  
  DateTime now = RTC.now();
  int hour = now.hour();
  bool pm = hour > 11;
  hour %= 12;

  line = formatString_P(PSTR("Time: %d:%0.2d:%0.2d %s"),
    hour == 0 ? 12 : hour,
    now.minute(),
    now.second(),
    pm ? "PM" : "AM");
  glcd.drawString(0, 0, line);
  
  if (currentState == States::timerPaused)
  {
    // Flash the "paused" text
    if (now.second() % 2 == 0)
    {
      glcd.drawString_P(0, 20, PSTR("Timer Paused"));
    }
  }
  
  long secondsElapsed = timer.GetElapsedSeconds();
  
  line = formatString_P(PSTR("%0.2ld:%0.2ld elapsed"), secondsElapsed / 60, secondsElapsed % 60);
  glcd.drawString(0, 30, line);
  
  long secondsRemaining = timer.GetTimespan() - secondsElapsed;
  line = formatString_P(PSTR("%0.2ld:%0.2ld remaining"), secondsRemaining / 60, secondsRemaining % 60);
  glcd.drawString(0, 40, line);

  glcd.drawString(0, 50, toString(freeRam()));
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

  switch (currentState)
  {
    case States::idle:
      if (alarmButtonDelta > 0)
      {
        timer.Reset();
        timer.Start();
        currentState = States::timerRunning;
      }
      
      if (encoderButtonDelta > 0)
      {
        GoToRootMenu();
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
          PauseTimer();
        }
        
        if (encoderButtonDelta > 0)
        {
          PauseTimer();
          GoToRootMenu();
        }
      }
      break;
      
    case States::timerRunningAndExpired:
      {
        if (alarmButtonDelta > 0 || encoderButtonDelta > 0)
        {
          PauseTimer();
          alarm.TurnOff();
          currentState = States::idle;
        }
      }
      break;
      
    case States::timerPaused:
      if (alarmButtonDelta > 0)
      {
        currentState = States::timerRunning;
        timer.Start();
      }
      
      if (encoderButtonDelta > 0)
      {
        GoToRootMenu();
      }
      break;
      
    case States::menu:      
      HandleMenuInput(alarmButtonDelta, encoderButtonDelta, encoderDelta);
      break;
  }
}

void PauseTimer()
{
  currentState = States::timerPaused;
  timer.Pause();
}

/*
ISR(WDT_vect)
{
  Sleepy::watchdogEvent();
}
*/

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
  lcdBacklight.SetColor(settings.lcdRed, settings.lcdGreen, settings.lcdBlue);
  
  // Ensure that the LCD is enabled.
  pinMode(Pins::lcdCS, OUTPUT);
  digitalWrite(Pins::lcdCS, LOW);

  glcd.begin(settings.lcdContrast);
  glcd.setFont(font_clR6x8);
  
  delay(500);
}

void configureRTC()
{
  Wire.begin();  
  RTC.begin();
  controlRTCSquareWave(false);
}

void controlRTCSquareWave(bool enable)
{
  RTC.controlSquareWaveOutput(enable, RTC_DS1307::OneHz, false);
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
  
  bool turnLightsOff = (currentMillis - lastLightsOnMillis) > Settings::lightsOffDelayMillis;
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


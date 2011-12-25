#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>

#include <JeeLib.h>
#include <GLCD_ST7565.h>
#include <avr/pgmspace.h>
#include <utility/font_clR6x8.h>
#include <utility/font_luBS18.h>

#include <RTClib.h>

#include <Tone.h>

#include "Pins.h"
#include "LCDBacklight.h"
#include "States.h"
#include "Timer.h"
#include "Alarm.h"
#include "Encoder.h"
#include "Button.h"
#include "Led.h"
#include "Piezo.h"
#include "Settings.h"
#include "Colors.h"

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

int currentState = States::idle;

volatile int squareCount;

int beforeMenuState;
int menuState;
int menuEncoderValue;
int currentMenuMax;

Settings settings;

char * formatString(char *format, ...);

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
  glcd.drawString(0, 0, toString(encoderCountRaw));
  glcd.drawString(0, 10, toString(encoderButtonPressCount));
  glcd.drawString(0, 20, toString(alarmButtonPressCount));
  glcd.drawString(0, 30, toString(timeDeltaMillis));
  glcd.drawString(0, 40, toString(squareCount));
  glcd.drawString(0, 50, "Elapsed Time: ");
  glcd.drawString(84, 50, toString(timer.GetElapsedSeconds()));
}

void DrawHomeScreen()
{
    char * line;
    
    DateTime now = RTC.now();
    int hour = now.hour();
    bool pm = hour > 11;
    hour %= 12;

    line = formatString("Time: %d:%0.2d:%0.2d %s",
      hour == 0 ? 12 : hour,
      now.minute(),
      now.second(),
      pm ? " PM" : " AM");
    glcd.drawString(0, 0, line);
    
    if (currentState == States::timerPaused)
    {
      // Flash the "paused" text
      if (now.second() % 2 == 0)
      {
        glcd.drawString(0, 20, "Timer Paused");
      }
    }
    
    long secondsElapsed = timer.GetElapsedSeconds();
    
    line = formatString("%0.2ld:%0.2ld elapsed", secondsElapsed / 60, secondsElapsed % 60);
    glcd.drawString(0, 30, line);
    
    long secondsRemaining = timer.GetTimespan() - secondsElapsed;
    line = formatString("%0.2ld:%0.2ld remaining", secondsRemaining / 60, secondsRemaining % 60);
    glcd.drawString(0, 40, line);
}

void DrawMenu()
{
  switch (menuState)
  {
    case 0:
      DrawRootMenu();
      break;
      
    case 1:
      DrawSetTimerMenu();
      break;
      
    case 2:
      DrawSetContrastMenu();
      break;
      
    case 3:
      DrawSetBacklightColorMenu();
      break;
  }
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

void HandleMenuInput(int alarmButtonDelta, int encoderButtonDelta, int encoderDelta)
{
  menuEncoderValue = min(currentMenuMax, max(0, menuEncoderValue + encoderDelta));
  
  switch (menuState)
  {
    case 0:
      if (alarmButtonDelta > 0)
      {
        currentState = beforeMenuState;
      }
      
      if (encoderButtonDelta > 0)
      {
        SetMenuState(menuEncoderValue + 1);
      }
      break;
      
    case 1:  // Set Timer
      if (encoderDelta != 0)
      {
        settings.timerMinutes = menuEncoderValue + 1;
      }
      
      if (alarmButtonDelta > 0)
      {
        SaveTimerMinutes(menuEncoderValue + 1);
        SetMenuState(0);
      }
      break;
      
    case 2: // Set Contrast
      if (encoderDelta != 0)
      {
        UpdateScreenContrast(menuEncoderValue + Settings::minLcdContrast);
      }
      
      if (alarmButtonDelta > 0)
      {
        SaveScreenContrast(menuEncoderValue + Settings::minLcdContrast);
        SetMenuState(0);
      }
      break;
      
    case 3:  // Set Backlight color
      if (encoderDelta != 0)
      {
        UpdateBacklightColor();
      }
      
      if (alarmButtonDelta > 0)
      {
        SaveBacklightColor();
        SetMenuState(0);
      }
      break;
  }
}

void SaveTimerMinutes(byte timerMinutes)
{
  settings.timerMinutes = timerMinutes;
  settings.writeTimerMinutes();
  timer.SetTimespan(timerMinutes * 60);
}

void SaveScreenContrast(byte contrast)
{
  settings.lcdContrast = contrast;
  settings.writeLcdContrast();
}

void UpdateScreenContrast(byte contrast)
{
  settings.lcdContrast = contrast;
  glcd.setContrast(contrast);
}

void SaveBacklightColor()
{
  Color color = Colors::GetColor(menuEncoderValue);
  settings.lcdRed = color.red;
  settings.lcdGreen = color.green;
  settings.lcdBlue = color.blue;
  settings.writeLcdColor();
}

void UpdateBacklightColor()
{
  Color color = Colors::GetColor(menuEncoderValue);
  lcdBacklight.SetColor(color.red, color.green, color.blue);
}

void PauseTimer()
{
  currentState = States::timerPaused;
  timer.Pause();
}

void GoToRootMenu()
{
  beforeMenuState = currentState;
  currentState = States::menu;
  SetMenuState(0);
}

void SetMenuState(int state)
{
  menuState = state;
  
  int numItems;

  switch (state)
  {
    case 0:
      menuEncoderValue = 0;
      numItems = 3;
      break;
    case 1:  // Set Timer
      menuEncoderValue = settings.timerMinutes;
      numItems = Settings::maxTimerMinutes - Settings::minTimerMinutes + 1;
      break;
    case 2:  // Set Contrast
      {
        numItems = Settings::maxLcdContrast - Settings::minLcdContrast + 1;
        menuEncoderValue = min(Settings::maxLcdContrast, max(Settings::minLcdContrast, settings.lcdContrast)) - Settings::minLcdContrast;
      }
      break;
    case 3:  // Set Backlight color
      menuEncoderValue = Colors::GetColorIndex(settings.lcdRed, settings.lcdGreen, settings.lcdBlue);
      if (menuEncoderValue < 0)
      {
        menuEncoderValue = 0;
      }
      numItems = Colors::NumColors();
      break;
  }
  
  currentMenuMax = numItems - 1;
}

void DrawRootMenu()
{
  drawMenuItem(0, "Set Timer", menuEncoderValue == 0);
  drawMenuItem(10, "Set Contrast", menuEncoderValue == 1);
  drawMenuItem(20, "Set Color", menuEncoderValue == 2);
}

void drawMenuItem(int y, char * text, bool selected)
{
  int x = glcd.drawString(7, y, text);
  if (selected)
  {
    glcd.fillTriangle(2, y, 2, y + 6, 5, y + 3, WHITE);
  }
}

void DrawSetTimerMenu()
{
  glcd.drawString(0, 0, "Setting Timer...");
  int x = glcd.drawString(0, 20, formatString("%d", settings.timerMinutes));
  if (RTC.now().second() % 2 == 0)
  {
    glcd.drawLine(0, 30, x, 30, WHITE);
  }
  
  glcd.drawString(x, 20, " minutes");
}

void DrawSetContrastMenu()
{
  static const int barWidth = 118;
  static const int barHeight = 10;
  
  float percentage = (float)(settings.lcdContrast - Settings::minLcdContrast) / (Settings::maxLcdContrast - Settings::minLcdContrast);
  int fillWidth = percentage * barWidth;
  
  glcd.drawString(0, 0, "Adjusting Contrast...");
  glcd.drawRect((LCDWIDTH - barWidth) / 2, (LCDHEIGHT - barHeight) / 2, barWidth, barHeight, WHITE);
  glcd.fillRect((LCDWIDTH - barWidth) / 2, (LCDHEIGHT - barHeight) / 2, fillWidth, barHeight, WHITE);
}

void DrawSetBacklightColorMenu()
{
  Color color = Colors::GetColor(menuEncoderValue);
  
  glcd.drawString(0, 0, "Setting Color...");
  glcd.drawString(0, 20, formatString("Color: %s", color.name));
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

char * formatString(char *format, ...)
{
  static char buf[50];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  
  return buf;
}

char * toString(int n)
{
  return formatString("%0.2d", n);
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


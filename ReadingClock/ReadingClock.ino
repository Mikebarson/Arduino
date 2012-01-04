#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <Tone.h>
#include <EEPROM.h>

#include <JeeLib.h>
#include <GLCD_ST7565.h>

#include "globals.h"
#include "Alarm.h"
#include "menu.h"
#include "utils.h"
#include "fonts.h"

static Alarm alarm(&UpdateAlarmOutputs);
static int currentState;
static int beforeMenuState;
static volatile uint32_t lastInputPulseCount = 0;

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
  
  EnablePulseSystem(true);
  GoToState(States::idle);
}

void loop()
{
  int timeDeltaMillis = getTimeDeltaMillis();

  adjustLights();

  timer.Update(timeDeltaMillis);
  alarm.Update(timeDeltaMillis);
  updateCurrentState();
  
  glcd.clear();
  
  switch (currentState)
  {
    case States::menu:
      DrawMenu();
      break;
      
    default:
      DrawHomeScreen();
      break;
  }
  
  if (SHOW_FREE_RAM)
  {
    DrawFreeRam();
  }
  
  glcd.refresh();
  
  // We don't power down while the alarm is sounding because we need
  // higher time precision -- when powered down, millis() stops working right.
  if (currentState != States::timerAlarmSounding)
  {
    Sleepy::powerDown();
  }
}

void DrawFreeRam()
{
  Fonts::SelectFont(Fonts::Small);
  const char *line = formatString_P(PSTR("%d bytes free"), freeRam());
  int textWidth = glcd.measureString(line);
  glcd.drawString(LCDWIDTH - textWidth - 1, LCDHEIGHT - glcd.textHeight() - 1, line);
}

void DrawDebuggingScreen(int timeDeltaMillis)
{
  Fonts::SelectFont(Fonts::Regular);
  glcd.drawString(0, 0, formatString_P(PSTR("Encoder: %0.2d"), encoderCountRaw));
  glcd.drawString(0, 10, formatString_P(PSTR("Encoder Button: %0.2d"), encoderButtonPressCount));
  glcd.drawString(0, 20, formatString_P(PSTR("Alarm Button: %0.2d"), alarmButtonPressCount));
  glcd.drawString(0, 30, formatString_P(PSTR("Time Delta:"), timeDeltaMillis));
  glcd.drawString(0, 40, formatString_P(PSTR("Elapsed Time: %d"), timer.GetElapsedSeconds()));
}

void DrawHomeScreen()
{
  static const int mainTextLeftOffset = 20;
  const char * line;
  byte textWidth;
  
  Fonts::SelectFont(Fonts::Small);
  
  glcd.drawString(0, 0, settings.ownerName);

  DateTime now = RTC.now();
  int hour = now.hour();
  bool pm = hour > 11;
  hour %= 12;

  line = formatString_P(PSTR("%d:%0.2d:%0.2d %s"),
    hour == 0 ? 12 : hour,
    now.minute(),
    now.second(),
    pm ? "PM" : "AM");
  textWidth = glcd.measureString(line);
  glcd.drawString(LCDWIDTH - textWidth - 1, 0, line);
  
  byte textHeight = glcd.textHeight();
  glcd.drawLine(0, textHeight, LCDWIDTH - 1, textHeight, WHITE);

  Fonts::SelectFont(Fonts::Regular);
  
  if (currentState == States::timerPaused)
  {
    // Flash the "paused" text
    if (pulseCount % 2 == 0)
    {
      glcd.drawString_P(mainTextLeftOffset, 20, PSTR("Timer Paused"));
    }
  }
  
  int secondsElapsed = timer.GetElapsedSeconds();
  line = formatString_P(PSTR("%0.2d:%0.2d elapsed"), secondsElapsed / 60, secondsElapsed % 60);
  glcd.drawString(mainTextLeftOffset, 30, line);
  
  int secondsRemaining = max(0, timer.GetTimespan() - secondsElapsed);
  line = formatString_P(PSTR("%0.2d:%0.2d remaining"), secondsRemaining / 60, secondsRemaining % 60);
  glcd.drawString(mainTextLeftOffset, 40, line);
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
        GoToState(States::timerResetting);
        GoToState(States::timerRunning);
      }
      
      if (encoderButtonDelta > 0)
      {
        GoToState(States::menu);
      }
      break;

    case States::timerRunning:
      {
        if (timer.IsExpired())
        {
          GoToState(States::timerAlarmSounding);
        }

        if (alarmButtonDelta > 0)
        {
          GoToState(States::timerPaused);
        }
        
        if (encoderButtonDelta > 0)
        {
          GoToState(States::timerPaused);
          GoToState(States::menu);
        }
      }
      break;

    case States::timerAlarmSounding:
      {
        if (alarmButtonDelta > 0 || encoderButtonDelta > 0)
        {
          GoToState(States::idle);
        }
      }
      break;

    case States::timerPaused:
      if (alarmButtonDelta > 0)
      {
        GoToState(States::timerRunning);
      }
      
      if (encoderButtonDelta > 0)
      {
        GoToState(States::menu);
      }
      break;

    case States::menu:
      if (HandleMenuInput(alarmButtonDelta, encoderButtonDelta, encoderDelta))
      {
        if (beforeMenuState == States::timerPaused &&
            timer.GetElapsedSeconds() == 0)
        {
          beforeMenuState = States::idle;
        }
        
        GoToState(beforeMenuState);
      }
      break;
  }
  
  GoToSleepIfAppropriate();
}

void GoToSleepIfAppropriate()
{
  static const uint32_t autoSleepPulseDeltaThreshold = Settings::autoSleepDelaySeconds * (uint32_t)PULSES_PER_SECOND;
    
  bool currentStateIsSleepable =
    currentState == States::idle ||
    currentState == States::timerPaused ||
    currentState == States::menu;

  uint32_t pulseDelta = pulseCount - lastInputPulseCount;

  if (!currentStateIsSleepable ||
      pulseDelta < autoSleepPulseDeltaThreshold)
  {
    return;
  }

  // Turn off the pulse system so it doesn't wake us back up.
  EnablePulseSystem(false);

  // Draw the sleeping screen contents, then turn off the LCD's control chip and its backlight
  DrawSleepingScreen();
  digitalWrite(Pins::lcdCS, HIGH);
  lcdBacklight.Disable();
  
  // Power down until we receive a hardware interrupt.
  Sleepy::powerDown();
  
  // Here is where we wake back up because of a hardware interrupt.
  // Turn the pulse back on.
  EnablePulseSystem(true);
  lastInputPulseCount = pulseCount;
  
  // Re-enable the LCD control chip
  digitalWrite(Pins::lcdCS, LOW);
}

void DrawSleepingScreen()
{
  glcd.clear();
  
  PGM_P sleepingText = PSTR("ZZZZZ.....");
  
  Fonts::SelectFont(Fonts::Small);
  int width = glcd.measureString_P(sleepingText);
  int height = glcd.textHeight();
  
  int x = ((LCDWIDTH - width) / 2) - 1;
  int y = ((LCDHEIGHT - height) / 2) - 1;
  glcd.drawString_P(x, y, sleepingText);

  glcd.refresh();
}

void GoToState(int state)
{
  switch (state)
  {
    case States::idle:
      timer.Pause();
      alarm.TurnOff();
      break;
      
    case States::timerResetting:
      timer.Reset();
      break;
      
    case States::timerRunning:
      timer.Start();
      break;
      
    case States::timerAlarmSounding:
      alarm.TurnOn();
      break;

    case States::timerPaused:
      timer.Pause();
      break;

    case States::menu:
      timer.Pause();
      alarm.TurnOff();
      beforeMenuState = currentState;
      currentState = States::menu;
      GoToRootMenu();
      break;
  }
  
  currentState = state;
}

ISR(WDT_vect)
{
  Sleepy::watchdogEvent();
}

ISR(PCINT2_vect)
{
  ++pulseCount;
  lastKnownMillis = millis();
}

ISR(PCINT1_vect)
{
  lastInputPulseCount = pulseCount;
  
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
  Fonts::SelectFont(Fonts::Regular);
  
  delay(500);
}

void configureRTC()
{
  Wire.begin();  
  RTC.begin();
}

void EnablePulseSystem(bool enable)
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

uint32_t getTimeDeltaMillis()
{
  static byte oldPulseCount = 0;
  
  byte nowPulseCount = pulseCount;
  byte deltaPulseCount = nowPulseCount - oldPulseCount;
  oldPulseCount = nowPulseCount;
  
  uint32_t nowMillis = millis();
  uint32_t deltaMillis = nowMillis - lastKnownMillis;
  lastKnownMillis = nowMillis;

  return (deltaPulseCount * MILLIS_PER_PULSE) + deltaMillis;
}

void adjustLights()
{
  static byte lastLightsOnPulseCount = 0;
  byte currentPulseCount = pulseCount;
  
  if (turnLightsOn)
  {
    lastLightsOnPulseCount = currentPulseCount;
    turnLightsOn = false;
    lcdBacklight.Enable();

    if (!alarm.IsOn())
    {
      alarmLed.Enable();
    }
    
    return;
  }
  
  uint32_t deltaMillis = (currentPulseCount - lastLightsOnPulseCount) * MILLIS_PER_PULSE;
  bool turnLightsOff = deltaMillis > (Settings::lightsOffDelaySeconds * 1000L);
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


#include <Arduino.h>
#include "Menu.h"
#include "Colors.h"
#include "States.h"
#include "globals.h"
#include "utils.h"

static int menuState;
static int menuEncoderValue;
static int currentMenuMax;

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

void drawMenuItem(int y, char * text, bool selected)
{
  int x = glcd.drawString(7, y, text);
  if (selected)
  {
    glcd.fillTriangle(2, y, 2, y + 6, 5, y + 3, WHITE);
  }
}

void DrawRootMenu()
{
  drawMenuItem(0, "Set Timer", menuEncoderValue == 0);
  drawMenuItem(10, "Set Contrast", menuEncoderValue == 1);
  drawMenuItem(20, "Set Color", menuEncoderValue == 2);
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

void GoToRootMenu()
{
  beforeMenuState = currentState;
  currentState = States::menu;
  SetMenuState(0);
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



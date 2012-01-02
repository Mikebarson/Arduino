#include <Arduino.h>
#include "Menu.h"
#include "Colors.h"
#include "States.h"
#include "globals.h"
#include "utils.h"

static int menuState;
static int currentMenuPosition;
static int currentMenuMin;
static int currentMenuNumItems;

struct MenuState
{
  enum Values
  {
    Root,
    Timer,
    Contrast,
    Color,
    Clock,
  };
};

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

void SaveBacklightColor(byte colorIndex)
{
  Color color = Colors::GetColor(colorIndex);
  settings.lcdRed = color.red;
  settings.lcdGreen = color.green;
  settings.lcdBlue = color.blue;
  settings.writeLcdColor();
}

void UpdateTimerMinutes(int timerMinutes)
{
  settings.timerMinutes = timerMinutes;
}

void UpdateScreenContrast(byte contrast)
{
  settings.lcdContrast = contrast;
  glcd.setContrast(contrast);
}

void UpdateBacklightColor(byte colorIndex)
{
  Color color = Colors::GetColor(colorIndex);
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
  drawMenuItem(0, "Set Timer", currentMenuPosition == MenuState::Timer);
  drawMenuItem(10, "Set Contrast", currentMenuPosition == MenuState::Contrast);
  drawMenuItem(20, "Set Color", currentMenuPosition == MenuState::Color);
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
  Color color = Colors::GetColor(currentMenuPosition);
  
  glcd.drawString(0, 0, "Setting Color...");
  glcd.drawString(0, 20, formatString("Color: %s", color.name));
}

void SetMenuState(int state)
{
  menuState = state;
  
  int numItems;

  switch (state)
  {
    case MenuState::Root:
      currentMenuMin = MenuState::Timer;
      currentMenuNumItems = 3;
      currentMenuPosition = currentMenuMin;
      break;

    case MenuState::Timer:
      currentMenuMin = Settings::minTimerMinutes;
      currentMenuNumItems = Settings::maxTimerMinutes - Settings::minTimerMinutes + 1;
      currentMenuPosition = min(Settings::maxTimerMinutes, max(Settings::minTimerMinutes, settings.timerMinutes));
      break;

    case MenuState::Contrast:
      currentMenuMin = Settings::minLcdContrast;
      currentMenuNumItems = Settings::maxLcdContrast - Settings::minLcdContrast + 1;
      currentMenuPosition = min(Settings::maxLcdContrast, max(Settings::minLcdContrast, settings.lcdContrast));
      break;

    case MenuState::Color:
      currentMenuMin = 0;
      currentMenuNumItems = Colors::NumColors();
      currentMenuPosition = Colors::GetColorIndex(settings.lcdRed, settings.lcdGreen, settings.lcdBlue);
      if (currentMenuPosition < 0)
      {
        currentMenuPosition = 0;
      }
      break;
  }
}

void DrawMenu()
{
  switch (menuState)
  {
    case MenuState::Root:
      DrawRootMenu();
      break;
      
    case MenuState::Timer:
      DrawSetTimerMenu();
      break;
      
    case MenuState::Contrast:
      DrawSetContrastMenu();
      break;
      
    case MenuState::Color:
      DrawSetBacklightColorMenu();
      break;
  }
}

void GoToRootMenu()
{
  beforeMenuState = currentState;
  currentState = States::menu;
  SetMenuState(MenuState::Root);
}

void HandleMenuInput(int alarmButtonDelta, int encoderButtonDelta, int encoderDelta)
{
  currentMenuPosition = min(currentMenuMin + currentMenuNumItems - 1, max(currentMenuMin, currentMenuPosition + encoderDelta));
  
  switch (menuState)
  {
    case MenuState::Root:
      if (alarmButtonDelta > 0)
      {
        currentState = beforeMenuState;
      }
      
      if (encoderButtonDelta > 0)
      {
        SetMenuState(currentMenuPosition);
      }
      break;
      
    case MenuState::Timer:
      if (encoderDelta != 0)
      {
        UpdateTimerMinutes(currentMenuPosition);
      }
      
      if (alarmButtonDelta > 0)
      {
        SaveTimerMinutes(currentMenuPosition);
        SetMenuState(MenuState::Root);
      }
      break;
      
    case MenuState::Contrast:
      if (encoderDelta != 0)
      {
        UpdateScreenContrast(currentMenuPosition);
      }
      
      if (alarmButtonDelta > 0)
      {
        SaveScreenContrast(currentMenuPosition);
        SetMenuState(MenuState::Root);
      }
      break;
      
    case MenuState::Color:
      if (encoderDelta != 0)
      {
        UpdateBacklightColor(currentMenuPosition);
      }
      
      if (alarmButtonDelta > 0)
      {
        SaveBacklightColor(currentMenuPosition);
        SetMenuState(MenuState::Root);
      }
      break;
  }
}


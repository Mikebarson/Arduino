#include <Arduino.h>
#include "Menu.h"
#include "Colors.h"
#include "States.h"
#include "globals.h"
#include "utils.h"

static int menuState;

void SetMenuState(int state);

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

class Menu
{
  public:
    int minPosition;
    int numPositions;
    int curPosition;

    virtual void Activate() {};
    virtual void OnEscape() {};
    virtual void OnEnter() {};
    virtual void OnScroll() {};
    virtual void Draw() = 0;
    
  protected:
    void drawMenuItem(int y, char * text, bool selected)
    {
      int x = glcd.drawString(7, y, text);
      if (selected)
      {
        glcd.fillTriangle(2, y, 2, y + 6, 5, y + 3, WHITE);
      }
    }
};

class RootMenu : public Menu
{
  public:
    RootMenu()
    {
      minPosition = MenuState::Timer;
      numPositions = 3;
      curPosition = minPosition;
    }
    
    virtual void Draw()
    {
      drawMenuItem(0, "Set Timer", curPosition == MenuState::Timer);
      drawMenuItem(10, "Set Contrast", curPosition == MenuState::Contrast);
      drawMenuItem(20, "Set Color", curPosition == MenuState::Color);
    }
    
    virtual void OnEscape()
    {
      currentState = beforeMenuState;
      curPosition = MenuState::Timer;
    }
        
    virtual void OnEnter()
    {
      SetMenuState(curPosition);
    }
};

class TimerMenu : public Menu
{
  public:
    TimerMenu()
    {
      ResetValues();
    }
    
    virtual void Activate()
    {
      ResetValues();
    }
    
    virtual void Draw()
    {
      glcd.drawString(0, 0, "Setting Timer...");
      int x = glcd.drawString(0, 20, formatString("%d", settings.timerMinutes));
      if (RTC.now().second() % 2 == 0)
      {
        glcd.drawLine(0, 30, x, 30, WHITE);
      }
      
      glcd.drawString(x, 20, " minutes");
    }
    
    virtual void OnScroll()
    {
      UpdateTimerMinutes(curPosition);
    }
  
    virtual void OnEscape()
    {
      SaveTimerMinutes(curPosition);
      SetMenuState(MenuState::Root);
    }
  
  private:
    void UpdateTimerMinutes(int timerMinutes)
    {
      settings.timerMinutes = timerMinutes;
    }
    
    void SaveTimerMinutes(byte timerMinutes)
    {
      settings.timerMinutes = timerMinutes;
      settings.writeTimerMinutes();
      timer.SetTimespan(timerMinutes * 60);
    }
    
    void ResetValues()
    {
      minPosition = Settings::minTimerMinutes;
      numPositions = Settings::maxTimerMinutes - Settings::minTimerMinutes + 1;
      curPosition = min(Settings::maxTimerMinutes, max(Settings::minTimerMinutes, settings.timerMinutes));
    }    
};

class ContrastMenu : public Menu
{
  public:
    ContrastMenu()
    {
      ResetValues();
    }
    
    virtual void Activate()
    {
      ResetValues();
    }
    
    virtual void Draw()
    {
      static const int barWidth = 118;
      static const int barHeight = 10;
      
      float percentage = (float)(settings.lcdContrast - Settings::minLcdContrast) / (Settings::maxLcdContrast - Settings::minLcdContrast);
      int fillWidth = percentage * barWidth;
      
      glcd.drawString(0, 0, "Adjusting Contrast...");
      glcd.drawRect((LCDWIDTH - barWidth) / 2, (LCDHEIGHT - barHeight) / 2, barWidth, barHeight, WHITE);
      glcd.fillRect((LCDWIDTH - barWidth) / 2, (LCDHEIGHT - barHeight) / 2, fillWidth, barHeight, WHITE);
    }
    
    virtual void OnScroll()
    {
      UpdateScreenContrast(curPosition);
    }

    virtual void OnEscape()
    {
      SaveScreenContrast(curPosition);
      SetMenuState(MenuState::Root);
    }
    
  private:
    void ResetValues()
    {
      minPosition = Settings::minLcdContrast;
      numPositions = Settings::maxLcdContrast - Settings::minLcdContrast + 1;
      curPosition = min(Settings::maxLcdContrast, max(Settings::minLcdContrast, settings.lcdContrast));
    }
    
    void UpdateScreenContrast(byte contrast)
    {
      settings.lcdContrast = contrast;
      glcd.setContrast(contrast);
    }

    void SaveScreenContrast(byte contrast)
    {
      settings.lcdContrast = contrast;
      settings.writeLcdContrast();
    }
};

class ColorMenu : public Menu
{
  public:
    ColorMenu()
    {
      ResetValues();
    }
    
    virtual void Activate()
    {
      ResetValues();
    }
    
    virtual void Draw()
    {
      Color color = Colors::GetColor(curPosition);
      
      glcd.drawString(0, 0, "Setting Color...");
      glcd.drawString(0, 20, formatString("Color: %s", color.name));
    }
    
    virtual void OnScroll()
    {
      UpdateBacklightColor(curPosition);
    }

    virtual void OnEscape()
    {
      SaveBacklightColor(curPosition);
      SetMenuState(MenuState::Root);
    }

  private:
    void ResetValues()
    {
      minPosition = 0;
      numPositions = Colors::NumColors();
      curPosition = Colors::GetColorIndex(settings.lcdRed, settings.lcdGreen, settings.lcdBlue);
      if (curPosition < 0)
      {
        curPosition = 0;
      }
    }
    
    void UpdateBacklightColor(byte colorIndex)
    {
      Color color = Colors::GetColor(colorIndex);
      lcdBacklight.SetColor(color.red, color.green, color.blue);
    }

    void SaveBacklightColor(byte colorIndex)
    {
      Color color = Colors::GetColor(colorIndex);
      settings.lcdRed = color.red;
      settings.lcdGreen = color.green;
      settings.lcdBlue = color.blue;
      settings.writeLcdColor();
    }
};

RootMenu rootMenu;
TimerMenu timerMenu;
ContrastMenu contrastMenu;
ColorMenu colorMenu;
Menu *currentMenu;

void SetMenuState(int state)
{
  menuState = state;

  switch (state)
  {
    case MenuState::Root:
      currentMenu = &rootMenu;
      break;

    case MenuState::Timer:
      currentMenu = &timerMenu;
      break;

    case MenuState::Contrast:
      currentMenu = &contrastMenu;
      break;

    case MenuState::Color:
      currentMenu = &colorMenu;
      break;
  }

  currentMenu->Activate();
}

void DrawMenu()
{
  currentMenu->Draw();
}

void GoToRootMenu()
{
  beforeMenuState = currentState;
  currentState = States::menu;
  SetMenuState(MenuState::Root);
}

void HandleMenuInput(int alarmButtonDelta, int encoderButtonDelta, int encoderDelta)
{
  currentMenu->curPosition = min(currentMenu->minPosition + currentMenu->numPositions - 1, max(currentMenu->minPosition, currentMenu->curPosition + encoderDelta));

  if (alarmButtonDelta > 0)
  {
    currentMenu->OnEscape();
  }

  if (encoderButtonDelta > 0)
  {
    currentMenu->OnEnter();
  }

  if (encoderDelta != 0)
  {
    currentMenu->OnScroll();
  }
}


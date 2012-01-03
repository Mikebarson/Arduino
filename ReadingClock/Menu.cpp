#include <Arduino.h>

#include "Menu.h"
#include "Colors.h"
#include "globals.h"
#include "fonts.h"
#include "utils.h"

void GoToMenu(int menuLevel);

struct MenuLevel
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
    int curPosition;
    
    virtual int MinPosition() = 0;
    virtual int NumPositions() = 0;
    
    virtual void Activate() {};
    virtual void OnEscape() {};
    virtual void OnEnter() {};
    virtual void OnScroll(int scrollDelta) {};
    virtual void Draw() = 0;
};

class RootMenu : public Menu
{
  public:
    RootMenu()
    {
      curPosition = MinPosition();
    }
    
    virtual int MinPosition() { return MenuLevel::Timer; }
    virtual int NumPositions() { return 4; }
    
    virtual void Draw()
    {
      drawMenuItem(0, PSTR("Set Timer"), curPosition == MenuLevel::Timer);
      drawMenuItem(10, PSTR("Screen Contrast"), curPosition == MenuLevel::Contrast);
      drawMenuItem(20, PSTR("Screen Color"), curPosition == MenuLevel::Color);
      drawMenuItem(30, PSTR("Adjust Clock"), curPosition == MenuLevel::Clock);
    }
    
    virtual void OnEscape()
    {
      currentState = beforeMenuState;
      curPosition = MinPosition();
    }
        
    virtual void OnEnter()
    {
      GoToMenu(curPosition);
    }

  private:
    void drawMenuItem(int y, PGM_P text, bool selected)
    {
      int x = glcd.drawString_P(7, y, text);
      if (selected)
      {
        glcd.fillTriangle(2, y, 2, y + 6, 5, y + 3, WHITE);
      }
    }
};

class TimerMenu : public Menu
{
  public:
    TimerMenu()
    {
      ResetValues();
    }
    
    virtual int MinPosition() { return Settings::minTimerMinutes; }
    virtual int NumPositions() { return Settings::maxTimerMinutes - Settings::minTimerMinutes + 1; }
    
    virtual void Activate()
    {
      ResetValues();
    }
    
    virtual void Draw()
    {
      glcd.drawString_P(0, 0, PSTR("Setting Timer..."));
      int x = glcd.drawString(0, 20, formatString_P(PSTR("%d"), settings.timerMinutes));
      if (RTC.now().second() % 2 == 0)
      {
        glcd.drawLine(0, 30, x, 30, WHITE);
      }
      
      glcd.drawString_P(x, 20, PSTR(" minutes"));
    }
    
    virtual void OnScroll(int scrollDelta)
    {
      UpdateTimerMinutes(curPosition);
    }
  
    virtual void OnEscape()
    {
      SaveTimerMinutes(curPosition);
      GoToMenu(MenuLevel::Root);
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
    
    virtual int MinPosition() { return Settings::minLcdContrast; }
    virtual int NumPositions() { return Settings::maxLcdContrast - Settings::minLcdContrast + 1; }
    
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
      
      glcd.drawString_P(0, 0, PSTR("Adjusting Contrast..."));
      glcd.drawRect((LCDWIDTH - barWidth) / 2, (LCDHEIGHT - barHeight) / 2, barWidth, barHeight, WHITE);
      glcd.fillRect((LCDWIDTH - barWidth) / 2, (LCDHEIGHT - barHeight) / 2, fillWidth, barHeight, WHITE);
    }
    
    virtual void OnScroll(int scrollDelta)
    {
      UpdateScreenContrast(curPosition);
    }

    virtual void OnEscape()
    {
      SaveScreenContrast(curPosition);
      GoToMenu(MenuLevel::Root);
    }
    
  private:
    void ResetValues()
    {
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
    
    virtual int MinPosition() { return 0; }
    virtual int NumPositions() { return Colors::NumColors(); }
    
    virtual void Activate()
    {
      ResetValues();
    }
    
    virtual void Draw()
    {
      Color color = Colors::GetColor(curPosition);
      
      glcd.drawString_P(0, 0, PSTR("Setting Color..."));
      int x = glcd.drawString_P(0, 20, PSTR("Color: "));
      glcd.drawString_P(x, 20, color.name);
    }
    
    virtual void OnScroll(int scrollDelta)
    {
      UpdateBacklightColor(curPosition);
    }

    virtual void OnEscape()
    {
      SaveBacklightColor(curPosition);
      GoToMenu(MenuLevel::Root);
    }

  private:
    void ResetValues()
    {
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

class ClockMenu : public Menu
{
  private:
    enum HourMinuteSecond
    {
      Hour,
      Minute,
      Second,
    };

    int editing;

  public:
    virtual int MinPosition() { return 0; }
    virtual int NumPositions() { return 60; }  // Irrelevant, since we handle the scroll increments directly.

    virtual void Draw()
    {
      glcd.drawString_P(0, 0, PSTR("Adjusting clock..."));

      DateTime now = RTC.now();
      int hour = now.hour();
      bool pm = hour > 11;
      hour %= 12;
      if (hour == 0)
      {
        hour = 12;
      }

      int lineStart = 0;
      int lineEnd = 0;
      int x;

      x = glcd.drawString(0, 20, formatString_P(PSTR("%d"), hour));
      lineEnd = (editing == Hour) ? x : lineEnd;
      x = glcd.drawString_P(x, 20, PSTR(":"));

      lineStart = (editing == Minute) ? x : lineStart;
      x = glcd.drawString(x, 20, toString(now.minute()));
      lineEnd  = (editing == Minute) ? x : lineEnd;

      x = glcd.drawString_P(x, 20, PSTR(":"));

      lineStart = (editing == Second) ? x : lineStart;
      x = glcd.drawString(x, 20, toString(now.second()));
      lineEnd = (editing == Second) ? x : lineEnd;

      x = glcd.drawString(x, 20, formatString_P(PSTR(" %s"), pm ? "PM" : "AM"));

      glcd.drawLine(lineStart, 30, lineEnd, 30, WHITE);
    }
    
    virtual void Activate()
    {
      editing = Hour;
      curPosition = RTC.now().hour();
    }

    virtual void OnEscape()
    {
      GoToMenu(MenuLevel::Root);
    }

    virtual void OnEnter()
    {
      DateTime now = RTC.now();
      
      switch (editing)
      {
        case Hour:
          editing = Minute;
          curPosition = now.minute();
          break;
          
        case Minute:
          editing = Second;
          curPosition = now.second();
          break;
          
        case Second:
          editing = Hour;
          curPosition = now.hour();
          break;
      }
    }

    virtual void OnScroll(int scrollDelta)
    {
      DateTime now = RTC.now();
      int hour = now.hour();
      int minute = now.minute();
      int second = now.second();
      
      switch (editing)
      {
        case Hour:
          hour = (hour + 24 + scrollDelta) % 24;
          break;

        case Minute:
          minute = (minute + 60 + scrollDelta) % 60;
          break;

        case Second:
          second = (second + 60 + scrollDelta) % 60;
          break;
      }
      
      RTC.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, second));
    }
};

RootMenu rootMenu;
TimerMenu timerMenu;
ContrastMenu contrastMenu;
ColorMenu colorMenu;
ClockMenu clockMenu;
Menu *currentMenu;

void GoToMenu(int state)
{
  switch (state)
  {
    case MenuLevel::Root:
      currentMenu = &rootMenu;
      break;

    case MenuLevel::Timer:
      currentMenu = &timerMenu;
      break;

    case MenuLevel::Contrast:
      currentMenu = &contrastMenu;
      break;

    case MenuLevel::Color:
      currentMenu = &colorMenu;
      break;

    case MenuLevel::Clock:
      currentMenu = &clockMenu;
      break;
  }

  currentMenu->Activate();
}

void DrawMenu()
{
  Fonts::SelectFont(Fonts::Regular);
  currentMenu->Draw();
}

void GoToRootMenu()
{
  beforeMenuState = currentState;
  currentState = States::menu;
  GoToMenu(MenuLevel::Root);
}

void HandleMenuInput(int alarmButtonDelta, int encoderButtonDelta, int encoderDelta)
{
  int minPosition = currentMenu->MinPosition();
  int maxPosition = minPosition + currentMenu->NumPositions() - 1;
  currentMenu->curPosition = min(maxPosition, max(minPosition, currentMenu->curPosition + encoderDelta));

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
    currentMenu->OnScroll(encoderDelta);
  }
}


#ifndef _settings_h
#define _settings_h

class Settings
{
  public:
    void readAllSettings();
    
    void writeTimerMinutes();
    void writeLcdContrast();
    void writeLcdColor();
    
    byte lcdContrast;
    byte lcdRed, lcdGreen, lcdBlue;
    byte timerMinutes;
    
    static const byte minLcdContrast;
    static const byte maxLcdContrast;
    
    static const byte minTimerMinutes;
    static const byte maxTimerMinutes;
    
    static const byte lightsOffDelaySeconds;
    static const int autoSleepDelaySeconds;
};

#endif // _settings_h


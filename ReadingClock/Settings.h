#ifndef _settings_h
#define _settings_h

class Settings
{
  public:
    void readAllSettings();
    
    void writeTimerMinutes();
    void writeLcdContrast();
    void writeLcdColor();
    void writeOwnerName(const char *ownerName);

    byte lcdContrast;
    byte lcdRed, lcdGreen, lcdBlue;
    byte timerMinutes;
    
    char ownerName[15];
    
    static const byte minLcdContrast;
    static const byte maxLcdContrast;
    
    static const byte minTimerMinutes;
    static const byte maxTimerMinutes;
    
    static const byte lightsOffDelaySeconds;
    static const int autoSleepDelaySeconds;
};

#endif // _settings_h


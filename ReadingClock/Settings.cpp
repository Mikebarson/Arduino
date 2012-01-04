#include <Arduino.h>
#include <EEPROM.h>
#include "Settings.h"

struct EEPROMAddresses
{
  enum Values
  {
    lcdContrast = 0,
    lcdRed = 1,
    lcdGreen = 2,
    lcdBlue = 3,
    timerMinutes = 4,
    ownerNameStart = 10,
  };
};

static const int defaultContrast = 0x18;
const byte Settings::minLcdContrast = 20;
const byte Settings::maxLcdContrast = 45;

static const int defaultTimerMinutes = 15;
const byte Settings::minTimerMinutes = 1;
const byte Settings::maxTimerMinutes = 120;

const byte Settings::lightsOffDelaySeconds = 10;
const int Settings::autoSleepDelaySeconds = 120;

void Settings::readAllSettings()
{
  lcdContrast = EEPROM.read(EEPROMAddresses::lcdContrast);
  if (lcdContrast < minLcdContrast ||
      lcdContrast > maxLcdContrast)
  {
    lcdContrast = defaultContrast;
  }
  
  lcdRed = EEPROM.read(EEPROMAddresses::lcdRed);
  lcdGreen = EEPROM.read(EEPROMAddresses::lcdGreen);
  lcdBlue = EEPROM.read(EEPROMAddresses::lcdBlue);
  
  timerMinutes = EEPROM.read(EEPROMAddresses::timerMinutes);
  if (timerMinutes < minTimerMinutes ||
      timerMinutes > maxTimerMinutes)
  {
    timerMinutes = defaultTimerMinutes;
  }

  int len = sizeof(ownerName) / sizeof(ownerName[0]);
  for (int i = 0; i < len; ++i)
  {
    ownerName[i] = EEPROM.read(EEPROMAddresses::ownerNameStart + i);
  }
}

void Settings::writeTimerMinutes()
{
  EEPROM.write(EEPROMAddresses::timerMinutes, timerMinutes);
}

void Settings::writeLcdContrast()
{
  EEPROM.write(EEPROMAddresses::lcdContrast, lcdContrast);
}

void Settings::writeLcdColor()
{
  EEPROM.write(EEPROMAddresses::lcdRed, lcdRed);
  EEPROM.write(EEPROMAddresses::lcdGreen, lcdGreen);
  EEPROM.write(EEPROMAddresses::lcdBlue, lcdBlue);
}

void Settings::writeOwnerName(const char *ownerName)
{
  int len = 0;
  for (int i = 0; ownerName[i] != 0; ++i)
  {
    EEPROM.write(EEPROMAddresses::ownerNameStart + i, ownerName[i]);
    ++len;
  }
  
  EEPROM.write(EEPROMAddresses::ownerNameStart + len, 0);
}


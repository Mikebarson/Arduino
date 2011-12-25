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
  };
};

static const int defaultContrast = 0x18;

const byte Settings::minLcdContrast = 20;
const byte Settings::maxLcdContrast = 45;

const byte Settings::minTimerMinutes = 1;
const byte Settings::maxTimerMinutes = 120;

const long Settings::lightsOffDelayMillis = 5000;

void Settings::readAllSettings()
{
  lcdContrast = EEPROM.read(EEPROMAddresses::lcdContrast);
  lcdRed = EEPROM.read(EEPROMAddresses::lcdRed);
  lcdGreen = EEPROM.read(EEPROMAddresses::lcdGreen);
  lcdBlue = EEPROM.read(EEPROMAddresses::lcdBlue);
  
  timerMinutes = EEPROM.read(EEPROMAddresses::timerMinutes);
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



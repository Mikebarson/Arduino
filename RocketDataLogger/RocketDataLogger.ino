#include <Wire.h>
#include <HP03M.h>
#include <SD.h>

#include "Pins.h"
#include "utils.h"

static char logFileName[16];

void setup()
{
  Serial.begin(115200);
  
  Hp03m.init(Pins::Barometer_XCLR);
  
  pinMode(Pins::LED_1, OUTPUT);
  pinMode(Pins::LED_2, OUTPUT);
  pinMode(Pins::LED_3, OUTPUT);
  pinMode(Pins::LED_Ground, OUTPUT);
  digitalWrite(Pins::LED_Ground, LOW);
  
  pinMode(Pins::Accel_X, INPUT);
  pinMode(Pins::Accel_Y, INPUT);
  pinMode(Pins::Accel_Z, INPUT);
  pinMode(Pins::Aux_Temperature, INPUT);
  
  InitializeSDCard();
  InitializeLogFileName();
  
  WriteToLog("Millis, Accel_X, Accel_Y, Accel_Z, BaroPressure, BaroTemp, AuxTemp");
}

static int activeLED = 0;

void loop()
{
  activeLED = (activeLED + 1) % 3;
  digitalWrite(Pins::LED_1, activeLED == 0 ? HIGH : LOW);
  digitalWrite(Pins::LED_2, activeLED == 1 ? HIGH : LOW);
  digitalWrite(Pins::LED_3, activeLED == 2 ? HIGH : LOW);
  
  uint32_t timestamp = millis();
  int accel_X = analogRead(Pins::Accel_X);
  int accel_Y = analogRead(Pins::Accel_Y);
  int accel_Z = analogRead(Pins::Accel_Z);
  int auxTemp = analogRead(Pins::Aux_Temperature);
  
  Hp03m.rawTemperaturePressureRead();
  
  const char *line = formatString("%ld, %d, %d, %d, %d, %d, %d", timestamp, accel_X, accel_Y, accel_Z, Hp03m.rawPressure, Hp03m.rawTemperature, auxTemp);
  WriteToLog(line);
  Serial.println(line);
}

void InitializeSDCard()
{
  Serial.print("Initializing SD card...");
  
  // see if the card is present and can be initialized:
  if (SD.begin(Pins::SD_ChipSelect))
  {
    Serial.println("card initialized.");
  }
  else
  {
    Serial.println("SD Card failed, or not present");
  }
}

void InitializeLogFileName()
{
  const char *candidate;
  
  for (int i = 0; i < 99; ++i)
  {
    candidate = formatString("data%d.csv", i);
    if (SD.exists((char *)candidate))
    {
      SD.remove((char *)candidate);
    }
  }
    
  for (int i = 0; i < 999; ++i)
  {
    candidate = formatString("data%d.csv", i);
    if (!SD.exists((char*)candidate))
    {
      strcpy(logFileName, candidate);
      Serial.println(formatString("Using %s as the logging file.", logFileName));
      return;
    }
  }

  Serial.println("Could not find a free log filename.  Giving up.");  
}

void WriteToLog(const char *line)
{
  File dataFile = SD.open(logFileName, FILE_WRITE);
  dataFile.println(line);
  dataFile.close();
}

int analogReadSensor(int pin)
{
  // Read once, and throw it away -- analog reads can be noisy for some sensors  
  analogRead(pin);
  return analogRead(pin);
}


#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <stdarg.h>

//#include "sms.h"
//SMSGSM sms;

static const int GPSRxPin = 6, GPSTxPin = 5;
static const int GSMPowerPin = 9;
static const uint32_t GPSBaud = 9600;
static const uint32_t GSMBaud = 19200;

SoftwareSerial gpsSerial(GPSRxPin, GPSTxPin);
Adafruit_GPS GPS(&gpsSerial);

double latitude;
double longitude;
double speed;
double angle;
short year;
byte month;
byte day;
byte hour;
byte minute;
byte second;
char mapUrl[128];

char smsbuffer[160];
char phoneNumber[20];

void SerialPrintf(char *fmt, ... ){
  char tmp[128]; // resulting string limited to 128 chars
  va_list args;
  va_start(args, fmt );
  vsnprintf(tmp, 128, fmt, args);
  va_end(args);
  Serial.print(tmp);
}

void setup()
{
  Serial.begin(115200);

  configureGPS();
  configureGSM();
  
  powerGSM();
     
  delay(1000);
}

void loop() // run over and over again
{
  readGPS();
}

void configureGPS()
{
  GPS.begin(9600);
  // Turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
}

void configureGSM()
{
  pinMode(GSMPowerPin, OUTPUT);
}

void powerGSM()
{
// software equivalent of pressing the GSM shield "power" button
  digitalWrite(GSMPowerPin, HIGH);
  delay(2000);
  digitalWrite(GSMPowerPin, LOW);
}

/*
void sendSMS()
{
  gsm.sendSMS("6512696432", "Howdy from Arduino.");
}

void readSMS()
{
  //Read if there are messages on SIM card and print them.
  if(gsm.readSMS(smsbuffer, 160, phoneNumber, 20))
  {
    Serial.println(phoneNumber);
    Serial.println(smsbuffer);
  }
}
*/


uint32_t timer = millis();
void readGPS()
{
  // read data from the GPS in the 'main loop'
  char c = GPS.read();

  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis()) timer = millis();
     
  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) {
    timer = millis(); // reset the timer
    
    if (GPS.fix)
    {
      year = 2000 + GPS.year;
      month = GPS.month;
      day = GPS.day;
      hour = GPS.hour;
      minute = GPS.minute;
      second = GPS.seconds;
    
      latitude = decodePositionValue(GPS.latitude * (GPS.lat == 'N' ? 1 : -1));
      longitude = decodePositionValue(GPS.longitude * (GPS.lon == 'E' ? 1 : -1));
      snprintf(mapUrl, 128, "https://maps.google.com/?q=%0ld.%06ld,%0ld.%06ld", (long)latitude, (long)(frac(latitude) * 1000000), (long)longitude, (long)(frac(longitude) * 1000000));
      speed = knotsToMPH(GPS.speed);
      angle = GPS.angle;
    
      SerialPrintf("\nTime: %02d:%02d:%02d\n", hour, minute, second);
      SerialPrintf("Date: %02d/%02d/%04d\n", month, day, year);
      SerialPrintf("Fix: %d quality: %d\n", (int)GPS.fix, GPS.fixquality);
      SerialPrintf("Url: %s\n", mapUrl);
      SerialPrintf("Speed (mph): %d\n", (int)speed);
      SerialPrintf("Angle: %0d.%02d\n", (int)angle, (int)(frac(angle) * 100));
  }
}

double frac(double value)
{
  value = fabs(value);
  value = fmod(value, (int)value);
  return value;
}

double knotsToMPH(double knots)
{
  return knots * 1.15078;
}

double decodePositionValue(double value)
{
  double sign = value / fabs(value);
  value = fabs(value);
  double degrees = floor(value / 100);
  double minutes = fmod(value, 100);
  
  value = sign * (degrees + (minutes / 60));
  return value;
}




#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 6, TXPin = 5;
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

double latitude;
double longitude;
double speed;
double course;
short year;
byte month;
byte day;
byte hour;
byte minute;
byte second;

void setup()
{
  Serial.begin(115200);
  gpsSerial.begin(GPSBaud);
}

void loop()
{
  while (gpsSerial.available() > 0)
  {
    if (gps.encode(gpsSerial.read()))
    {
      boolean haveValidGPS = gps.location.isValid() && gps.location.age() < 1500;
      if (haveValidGPS)
      {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        speed = gps.speed.mph();
        course = gps.course.deg();
        year = gps.date.year();
        month = gps.date.month();
        day = gps.date.day();
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();
      }
    }
  }
}


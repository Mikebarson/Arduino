#include <Servo.h>
#include <Wire.h>
#include <RTClib.h>
 
RTC_DS1307 RTC;
Servo myservo;

const int servoSignalPin = 9;

const int offPosition = 98;
const int bluePosition = 128;
const int whitePosition = 58;

int currentPosition;

struct TimeAndPosition
{
  TimeAndPosition(int _hour, int _minute, int _position)
  {
    hour = _hour;
    minute = _minute;
    position = _position;
  }

  int hour;
  int minute;  
  int position;
};

TimeAndPosition* timesAndPositions[] =
{
  // The entries should be sorted by time.
  new TimeAndPosition(9, 0, bluePosition),
  new TimeAndPosition(11, 0, whitePosition),
  new TimeAndPosition(17, 0, bluePosition),
  new TimeAndPosition(19, 0, offPosition),
};

void setup() 
{
  Serial.begin(57600);
  Wire.begin();

  // Configure pins to power the RTC
  pinMode(A3, OUTPUT);
  digitalWrite(A3, HIGH);
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);

  RTC.begin();

  if (!RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  myservo.attach(servoSignalPin);
  
  positionServo(offPosition);
} 

void positionServo(int position)
{
    currentPosition = position;
    myservo.write(position);
}

int getPositionForTime(DateTime time)
{
  int length = sizeof(timesAndPositions) / sizeof(timesAndPositions[0]);
  
  // Walk the list backwards, since it's sorted from early to late.
  for (int i = length - 1; i >= 0; --i)
  {
    TimeAndPosition *target = timesAndPositions[i];
    if ((time.hour() > target->hour) ||
        (time.hour() == target->hour && time.minute() >= target->minute))
    {
      return target->position;
    }
  }
  
  return offPosition;
}

void loop()
{
  DateTime now = RTC.now();
  int position = getPositionForTime(now);
  
  if (currentPosition != position)
  {
    positionServo(position);
  }
  
  delay(30000);  // Sleep for 30 seconds.
}



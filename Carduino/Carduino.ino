#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

static const int GPSRxPin = 6, GPSTxPin = 5;
static const int GSMRxPin = 7, GSMTxPin = 8;
static const int GSMPowerPin = 9;
static const uint32_t GPSBaud = 9600;
static const uint32_t GSMBaud = 19200;

SoftwareSerial gpsSerial(GPSRxPin, GPSTxPin);
SoftwareSerial gsmSerial(GSMRxPin, GSMTxPin);
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

char smsBuffer[160];
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

  Serial.println("configuring gps");
  configureGPS();
  Serial.println("configuring gsm");
  configureGSM();
}

void loop()
{
  Serial.println("listening to gps");
  gpsSerial.listen();
  readGPS();

  Serial.println("listening to sms");
  gsmSerial.listen();
  readSMS();
}

void configureGPS()
{
  GPS.begin(GPSBaud);
  // Turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
}

void configureGSM()
{
  gsmSerial.begin(GSMBaud);
  pinMode(GSMPowerPin, OUTPUT);  

  Serial.print("Powering on the GSM module... ");
  powerOnGSM();
  Serial.println("Done.");

  Serial.print("Connecting to the GSM network... ");
  while( (sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || 
    sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0 );
  Serial.println("Done.");

  Serial.print("Setting SMS mode... ");
  sendATcommand("AT+CMGF=1", "OK", 1000);    // sets the SMS mode to text
  Serial.println("Done.");

  Serial.print("Configuring SMS storage... ");
  sendATcommand("AT+CPMS=\"SM\",\"SM\",\"SM\"", "OK", 1000);    // selects the memory
  Serial.println("Done.");
}

void powerOnGSM()
{
  uint8_t answer = 0;

  // checks if the module is started
  answer = sendATcommand("AT", "OK", 2000);
  if (answer == 0)
  {
    // power on pulse
    digitalWrite(GSMPowerPin,HIGH);
    delay(3000);
    digitalWrite(GSMPowerPin,LOW);

    // waits for an answer from the module
    while (answer == 0)
    {
      // Send AT every two seconds and wait for the answer
      answer = sendATcommand("AT", "OK", 2000);
    }
  }
}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout)
{
  uint8_t x=0,  answer=0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Initialice the string

  delay(100);

  while (gsmSerial.available() > 0) gsmSerial.read();    // Clean the input buffer

  gsmSerial.println(ATcommand);    // Send the AT command 

    x = 0;
  previous = millis();

  // this loop waits for the answer
  do
  {
    // if there are data in the UART input buffer, reads it and checks for the asnwer
    if (gsmSerial.available() != 0){    
      response[x] = gsmSerial.read();
      Serial.print(response[x]);
      x++;
      // check if the desired answer is in the response of the module
      if (strstr(response, expected_answer) != NULL)    
      {
        answer = 1;
      }
    }
    // Waits for the asnwer with time out
  } 
  while ((answer == 0) && ((millis() - previous) < timeout));    

  return answer;
}

void sendSMS()
{
  char *phone_number = "+16512696432";
  char *message = "Test-Arduino-Hello World";

  Serial.println("Sending SMS");

  char aux_string[30];
  sprintf(aux_string,"AT+CMGS=\"%s\"", phone_number);
  byte answer = sendATcommand(aux_string, ">", 2000);    // send the SMS number
  if (answer == 1)
  {
    gsmSerial.println(message);
    gsmSerial.write(0x1A);
    answer = sendATcommand("", "OK", 20000);
    if (answer == 1)
    {
      Serial.print("Sent");    
    }
    else
    {
      Serial.print("error");
    }
  }
  else
  {
    Serial.print("error");
    Serial.println(answer, DEC);
  }
}

void readSMS()
{
    char answer = sendATcommand("AT+CMGR=1", "+CMGR:", 2000);    // reads the first SMS
    if (answer == 1)
    {
      Serial.println("Checking for sms message...");
      
      int i = 0;
      i = readGSMUntil(i, "\r\n");
      i = readGSMUntil(i, "\r\n");
      i = readGSMUntil(i, "OK");

      smsBuffer[i] = '\0';
      Serial.print(smsBuffer);
    }
    else
    {
      Serial.print("error ");
      Serial.println(answer, DEC);
    }
}

int readGSMUntil(int i, char *chars)
{
  int charIndex = 0;
  char c;
  while (chars[charIndex] != '\0')
  {
    if (c = gsmSerial.read())
    {
      smsBuffer[i++] = c;
      
      if (c == chars[charIndex])
      {
        charIndex++;
      }
    }
  }
  
  return i;
}

boolean StartsWith(const char *a, const char *b)
{
  if(strncmp(a, b, strlen(b)) == 0) return 1;
  return 0;
}

void readGPS()
{
  boolean validGGAReceived = false;
  boolean validRMCReceived = false;

  while (!validGGAReceived || !validRMCReceived)
  {
    GPS.read();
    if (GPS.newNMEAreceived()) {
      char *nmea = GPS.lastNMEA();
      if (StartsWith(nmea, "$GPGGA"))
      {
        validGGAReceived = GPS.parse(nmea);
      }
      else if (StartsWith(nmea, "$GPRMC"))
      {
        validRMCReceived = GPS.parse(nmea);
      }
    }
  }

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

  SerialPrintf("Time: %02d:%02d:%02d\n", hour, minute, second);
  SerialPrintf("Date: %02d/%02d/%04d\n", month, day, year);
  SerialPrintf("Fix: %d quality: %d\n", (int)GPS.fix, GPS.fixquality);
  SerialPrintf("Url: %s\n", mapUrl);
  SerialPrintf("Speed (mph): %d\n", (int)speed);
  SerialPrintf("Angle: %0d.%02d\n", (int)angle, (int)(frac(angle) * 100));
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






#include <Arduino.h>

#include <Wire.h>
#include <RTClib.h>

// Demo for multiple fonts on the Graphics Board
// 2010-05-28 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <GLCD_ST7565.h>
#include <JeeLib.h>
#include <avr/pgmspace.h>

#include "utility/font_clR6x8.h"
#include "utility/font_clR6x6.h"
#include "utility/font_helvBO12.h"
#include "utility/font_helvO12.h"
#include "utility/font_clR4x6.h"
#include "utility/font_ncenBI14.h"
#include "utility/font_ncenR08.h"
#include "utility/font_luBS19.h"

GLCD_ST7565 glcd;
RTC_DS1307 RTC;

char * toString(int n)
{
  static char buf[24];

  snprintf(buf, 24, "%d", n);
  return buf;
}

void setup () {
    Wire.begin();
    RTC.begin();
  
  glcd.begin(0x18);
  glcd.setFont(font_luBS19);
//  glcd.setFont(font_clR4x6);
  delay(500);
}

void loop ()
{
  glcd.clear();

  DateTime now = RTC.now();
  int x = glcd.drawString(0, 0, toString(now.hour()));
  x = glcd.drawString(x, 0, ":");
  x = glcd.drawString(x, 0, toString(now.minute()));  
  x = glcd.drawString(x, 0, ":");
  glcd.drawString(x, 0, toString(now.second()));  

  glcd.refresh();
}


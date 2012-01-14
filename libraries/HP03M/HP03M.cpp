/*
  HO03M.cpp - HP03M barometer library
  2010 Copyright (c) Seeed Technology Inc.  All right reserved.
 
  Original Author: Albert.Miao
  Contribution: Visweswara R (API redesign, Change to Arduino library format,fix for lookup table)
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "HP03M.h"
#include <avr/pgmspace.h>

#if ARDUINO >= 100
#	define Wire_write Wire.write
#	define Wire_read Wire.read
#else
#	define Wire_write Wire.send
#	define Wire_read Wire.receive
#endif

/*
IMPORTANT NOTE: The below lookup table is placed in program memory. Hence PROGMEM is used.
The problem without using PROGMEM will not be visible until RAM usage is more in the application.
*/

const long HP03M::Tab_BasicAltitude[80] PROGMEM=
{
    -6983,-6201,-5413,-4620,-3820,-3015,-2203,-1385,-560, 270,    //0.1m
    //  1100  1090  1080  1070  1060  1050 1040 1030 1020 1010    //hpa
    1108, 1953, 2805, 3664, 4224, 5403, 6284, 7172, 8068, 8972,
    //  1000  990   980   970   960   950  940  930  910  910     //hpa
    9885,10805,11734,12671,13617,14572,15537,16510,17494,18486,
    //  900   890   880   870   860   850  840  830  820  810     //hpa
    19489,20502,21526,22560,23605,24662,25730,26809,27901,29005,
    //  800   790   780   770   760   750  740  730  720  710     //hpa
    30121,31251,32394,33551,34721,35906,37106,38322,39553,40800,
    //  700   690   680   670   660   650  640  630  620  610     //hpa
    42060,43345,44644,45961,47296,48652,50027,51423,52841,54281,
    //  600   590   580   570   560   550  540  530  520  510     //hpa
    55744,57231,58742,60280,61844,63436,65056,66707,68390,70105,
    //  500   490   480   470   460   450  440  430  420  410     //hpa
    71854,73639,75461,77323,79226,81172,83164,85204,87294,89438
    //400   390   380   370   360   350  340  330  320  310       //hpa
};  // write to program memory / flash

/* 
 * Function coefficientFill
 * Desc     The raw calibration data read is filled into individual coefficient variables
 * Input    Pointer to coefficient data array
 * Output   none
 */
void HP03M::coefficientFill(unsigned char *Coefficient)
{
    C1=(Coefficient[0]<<8)+Coefficient[1];
    C2=(Coefficient[2]<<8)+Coefficient[3];
    C3=(Coefficient[4]<<8)+Coefficient[5];
    C4=(Coefficient[6]<<8)+Coefficient[7];
    C5=(Coefficient[8]<<8)+Coefficient[9];
    C6=(Coefficient[10]<<8)+Coefficient[11];
    C7=(Coefficient[12]<<8)+Coefficient[13];
    AA=Coefficient[14];
    BB=Coefficient[15];
    CC=Coefficient[16];
    DD=Coefficient[17];

#if HP03M_Debug&0b00000001
    Serial.print("Debug Coefficient");
    Serial.print("C1=");
    Serial.println(C1);
    Serial.print("C2=");
    Serial.println(C2);
    Serial.print("C3=");
    Serial.println(C3);
    Serial.print("C4=");
    Serial.println(C4);
    Serial.print("C5=");
    Serial.println(C5);
    Serial.print("C6=");
    Serial.println(C6);
    Serial.print("C7=");
    Serial.println(C7);
    Serial.print("AA=");
    Serial.println(AA,DEC);
    Serial.print("BB=");
    Serial.println(BB,DEC);
    Serial.print("CC=");
    Serial.println(CC,DEC);
    Serial.print("DD=");
    Serial.println(DD,DEC);
#endif
}

/* 
 * Function calibrationDataRead
 * Desc     Read the calibration data from HP03M EEPROM
 * Input    none
 * Output   none
 */
void HP03M::calibrationDataRead(void)
{
    unsigned char calibrationData[18];
    digitalWrite(xclr_pin,LOW);
    Wire.beginTransmission(HP03M_CalEEPROM_Address);
    Wire_write(0x10);
    Wire.endTransmission();
    for(unsigned char i=0;i<18;i++)
    {
        Wire.requestFrom(HP03M_CalEEPROM_Address,1);
        while(Wire.available())
        {
            calibrationData[i]= Wire_read();
        }
        
    }
    coefficientFill(calibrationData);
    return;
}

/* 
 * Function rawPressureReadHelper
 * Desc     Helper function to Read uncompensated Pressure data from HP03M ADC
 * Input    pointer to Pressure data
 * Output   none
 */
void HP03M::rawPressureReadHelper(unsigned char *pressure) 
{
    Wire.beginTransmission(HP03M_ADC_Address);
    Wire_write(0xFF);
    Wire_write(0xF0);
    Wire.endTransmission();
    delay(40);
    Wire.beginTransmission(HP03M_ADC_Address);
    Wire_write(0xFD);
    Wire.endTransmission();
    for(unsigned char i=0;i<2;i++)
    {
        Wire.requestFrom(HP03M_ADC_Address,1);
        while(Wire.available())
        {
            pressure[i]= Wire_read();
        }
    }
    Wire.endTransmission(); //End read, got the Pressure data
}

/* 
 * Function rawTemperatureReadHelper
 * Desc     Helper function to Read uncompensated Temperature data from HP03M ADC
 * Input    Pointer to Temperature data
 * Output   none
 */

void rawTemperatureReadHelper(unsigned char *temperature)
{
    Wire.beginTransmission(HP03M_ADC_Address);
    Wire_write(0xFF);
    Wire_write(0xE8);
    Wire.endTransmission();
    delay(40);
    Wire.beginTransmission(HP03M_ADC_Address);
    Wire_write(0xFD);
    Wire.endTransmission();
    for(unsigned char i=0;i<2;i++)
    {
        Wire.requestFrom(HP03M_ADC_Address, 1 );
        while(Wire.available())
        {
            temperature[i]= Wire_read();
        }
    }
    Wire.endTransmission(); //End read, got the Temperature data
}

/* 
 * Function rawTemperaturePressureRead
 * Desc     Read uncompensated Pressure and Temperature data from HP03M ADC
 * Input    none
 * Output   Temperature and Pressure
 */
void HP03M::rawTemperaturePressureRead(void)
{
    unsigned char cache_rawPressure[2];
    unsigned char cache_rawTemperature[2];
    digitalWrite(xclr_pin,HIGH);
    rawPressureReadHelper(cache_rawPressure);
    rawTemperatureReadHelper(cache_rawTemperature);// read out 16bit rawTemperature
    digitalWrite(xclr_pin,LOW);
    rawPressure=(cache_rawPressure[0]<<8)+cache_rawPressure[1];
    rawTemperature=(cache_rawTemperature[0]<<8)+cache_rawTemperature[1];

#if HP03M_Debug&0b00000010
    //debug info
    Serial.print("rawPressure=");
    Serial.println(rawPressure);
    Serial.print("rawTemperature=");
    Serial.println(rawTemperature);
#endif
}

/* 
 * Function realTemperaturePressureCalculate
 * Desc     Calculate Real Pressure and Temperature by compensating with coefficient data.
 * Input    none
 * Output   Real Pressure(unit: hPa) and Real Temperature (unit: °C) 
 */
void HP03M::realTemperaturePressureCalculate(void)
{
    float DUT ;
    float OFF;
    float SENS;
    float X;
    float MiddleData1;
    float MiddleData2;
    float MiddleData3;
    float MiddleData4;


    if(rawTemperature < C5)
    {
        //DUT = rawTemperature-C5-((rawTemperature-C5)/Get2_x(7))*((rawTemperature-C5)/Get2_x(7))*BB/Get2_x(CC);
        MiddleData1 = (float)rawTemperature-(float)C5;
        MiddleData2 = MiddleData1*MiddleData1/16384;
        MiddleData3 = MiddleData2*BB;
        MiddleData4 = left_shift_bits(CC);
        MiddleData4 = MiddleData3/MiddleData4;
        DUT = MiddleData1 - MiddleData4;
    }
    else
    {
        //DUT = rawTemperature-C5-((rawTemperature-C5)/Get2_x(7))*((rawTemperature-C5)/Get2_x(7))*AA/Get2_x(C);
        MiddleData1 = (float)rawTemperature-(float)C5;
        MiddleData2 = MiddleData1*MiddleData1/16384;
        MiddleData3 = MiddleData2*AA;
        MiddleData4 = left_shift_bits(CC);
        MiddleData4 = MiddleData3/MiddleData4;
        DUT = MiddleData1 - MiddleData4;
    }


    //calculate the OFF value
    //OFF = (C2+(C4-1024)*D UT/Get2_x(14))*4;
    MiddleData1 = (float)C4-1024;
    MiddleData2 = left_shift_bits(14);
    MiddleData3 = DUT*MiddleData1;
    MiddleData4 = MiddleData3/MiddleData2;
    MiddleData4 = (float)C2+MiddleData4;
    OFF = MiddleData4*4;

    //calculate the SENS value
    //SENS = C1+C3*DUT/Get2_x(10);
    MiddleData1 = (float)C3*DUT;
    MiddleData2 = left_shift_bits(10);
    MiddleData3 = MiddleData1/ MiddleData2;
    SENS = C1+MiddleData3;

    //calculate the X value
    //X = SENS*(rawPressure-7168)/ Get2_x(14)-OFF;
    MiddleData1 = left_shift_bits(14);
    MiddleData2 = (float)rawPressure-7168;
    MiddleData3 = MiddleData2*SENS;

    MiddleData4 = MiddleData3/MiddleData1;
    X = MiddleData4-OFF;
    //calculate the Pressure value,have two decimal fraction
    //Pressure = X*100/Get2_x(5)+C7*10;
    MiddleData1 = X*100;
    MiddleData2 = left_shift_bits(5);
    MiddleData3 = MiddleData1/MiddleData2;
    MiddleData4 = C7*10;
    Pressure = MiddleData3+MiddleData4;

    //calculate the Temperature value
    Temperature = 250+((DUT*C6)/65536)-(DUT/left_shift_bits(DD));
    Temperature=Temperature/10;


#if HP03M_Debug&0b00000100
    Serial.print("DUT =");           // print the character
    Serial.println(DUT);             // print the character
    Serial.print("Press =");         // print the character
    Serial.println(Pressure);        // print the character
    Serial.print("Temperature =");   // print the character
    Serial.println(Temperature);     // print the character
#endif
}

/* 
 * Function altitudeCalculate
 * Desc     Calculate altitide from Pressure.
 * Input    Real Pressure value
 * Output   Altitude, unit: 0.1m 
 */
void HP03M::altitudeCalculate(void)
{ 
    unsigned char ucCount;
    unsigned int  uiBasicPress;
    unsigned int  uiBiasTotal;
    unsigned int  uiBiasPress;
    unsigned int  uiBiasAltitude;
    unsigned int  uiPress=Pressure/100;
 

    for(ucCount=0;;ucCount++)
    {
        uiBasicPress = MaxPress-(ucCount*10);
        if(uiBasicPress < uiPress)
        {
            break;
        }
    }

    uiPress=    (long)pgm_read_dword(&Tab_BasicAltitude[ucCount]);
    uiBiasTotal = ((long)pgm_read_dword(&Tab_BasicAltitude[ucCount])) - ((long)pgm_read_dword(&Tab_BasicAltitude[ucCount-1]));
    uiBiasPress = Pressure - (long)(uiBasicPress*100);
    uiBiasAltitude = (long)uiBiasTotal * (long)uiBiasPress/1000;
    Altitude = ((long)pgm_read_dword(&Tab_BasicAltitude[ucCount])) - (long)uiBiasAltitude;
    ucCount = (abs(Altitude % 10));//round off
    if(Altitude < 0)
    {
        if(ucCount > 4)
            Altitude -= 10-ucCount;
    }
    else
    {
        if(ucCount>4)
            Altitude += 10-ucCount;
        else
            Altitude -= ucCount;
    }
    Altitude=Altitude/10;

#if HP03M_Debug&0b00001000
    Serial.print("Altitude =");
    Serial.println(Altitude);
#endif
}


/* 
 * Function read
 * Desc     Read pressure,temperature,altitidue from HP03M
 * Input    None
 * Output   Temperature, Pressure, Altitidue
 */
void HP03M::read(void)
{
    rawTemperaturePressureRead();
    realTemperaturePressureCalculate();
    altitudeCalculate();
#if HP03M_Debug&0b00001000
    Serial.print("Temperature=");
    Serial.println(Temperature);
    Serial.print("Altitude=");
    Serial.println(Altitude);
#endif
}

/* 
 * Function init
 * Desc     initialize HP03M sensor
 * Input    None
 * Output   None
 */
void HP03M::init(int xclr_pin)
{
    this->xclr_pin = xclr_pin;
    Wire.begin();
    pinMode(xclr_pin,OUTPUT);
    delay(100);	
    calibrationDataRead();
}

HP03M Hp03m;  // Preinstantiate Objects 

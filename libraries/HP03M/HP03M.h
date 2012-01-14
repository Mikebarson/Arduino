/*
  HP03M.h - HP03M barometer library
  2010 Copyright (c) Seeed Technology Inc.  All right reserved.
 
  Original Author: Albert.Miao
  Contribution: Visweswara R (API redesign, Change to Arduino library format)
  
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

#ifndef HP03M_data_H
#define HP03M_data_H

#define HP03M_Debug             0b00000000
#define HP03M_CalEEPROM_Address 0x50    // 0xa0>>1 The Wire library will left shift 1 bit and control the LSB first bit
#define HP03M_ADC_Address       0x77    // 0xee>>1
#define MaxPress                1100    // 1100Hpa
#define left_shift_bits(x)     (1<<x)

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <Wire.h>


class HP03M {

public:
int xclr_pin;
long Altitude;
float Pressure;
float Temperature;
unsigned int rawPressure; // measured pressure
unsigned int rawTemperature; // measured temperature

void init(int xclr_pin = 4);
void read(void);
void realTemperaturePressureCalculate(void); // Calculate real temperature and pressure from raw value read from sensor
void rawTemperaturePressureRead(void);
void altitudeCalculate(void);                // Calculate altitude from real atmospheric atmospheric pressure  
void calibrationDataRead(void);              // Read the calibration data from HP03M EEPROM

private:
unsigned int C1; // Sensitivity coefficient
unsigned int C2; // Offset coefficient
unsigned int C3; // Temperature Coefficient of Sensitivity
unsigned int C4; // Temperature Coefficient of Offset
unsigned int C5; // Reference Temperature
unsigned int C6; // Temperature Coefficient of Temperature
unsigned int C7; // Offset Fine Tuning
unsigned char AA, BB, CC, DD; // Sensor specific parameter A,B,C,D
static const long Tab_BasicAltitude[80];

void coefficientFill(unsigned char *Coefficient);
void rawPressureReadHelper(unsigned char *Pressure);

};

extern HP03M Hp03m;  // HP03M object 

#endif

/*
SeeedStudio Motion Frame Demo code.
2010 Copyright (c) Seeed Technology Inc.  All right reserved.
Original Author: Albert.Miao
Contribution: Visweswara R (API redesign, Change to Arduino library format)
   
This demo code is free software; you can redistribute it and/or
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

For more details about the product please check http://www.seeedstudio.com/depot/
*/

#include <HP03M.h>
#include <Wire.h>


void setup()  
{
    Serial.begin(9600);
     Hp03m.init();      //Initialize Hp03m  

}

void loop()
{
    Hp03m.read();  // Read temperature, pressure and altitude
    
    Serial.print("Temperature=");
    Serial.println(Hp03m.Temperature);
    
    Serial.print("Pressure=");
    Serial.println(Hp03m.Pressure);
    
    Serial.print("Altitude=");
    Serial.println(Hp03m.Altitude);
    
    delay(1000);
}

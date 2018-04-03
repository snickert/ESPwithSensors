# ESPwithSensors
Connect a Dallas-Temperature Sensor and a Color-Sensor to an ESP32 with a WLAN connection 

In this Program a combination of serveral RGB-Sensors TCS34725 and Temperature-Sensors DS18B20
are reading the actual temperature as well as the actual RBG-Values and send them in a HTTP-Post over WLAN to a
Server in form of JSON-Packages

The TCS34725 and DS18B20 librarys can also find here on GitHub if further documentation is needed

For ur information:

## Adafruit TCS34725 Color Sensor Driver ##

This driver is for the Adafruit TCS34725 Breakout.
    ------> http://www.adafruit.com/products/1334

## About this Driver ##

These modules use I2C to communicate, 2 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Kevin (KTOWN) Townsend for Adafruit Industries.
BSD license, check license.txt for more information
All text above must be included in any redistribution

To download. click the ZIP button in the top bar, and check this tutorial
for instructions on how to install: 
http://learn.adafruit.com/adafruit-all-about-arduino-libraries-install-use


# Arduino Library for Maxim Temperature Integrated Circuits

## Usage

This library supports the following devices :


* DS18B20
* DS18S20 - Please note there appears to be an issue with this series.
* DS1822
* DS1820
* MAX31820


You will need a pull-up resistor of about 5 KOhm between the 1-Wire data line
and your 5V power. If you are using the DS18B20, ground pins 1 and 3. The
centre pin is the data line '1-wire'.

We have included a "REQUIRESNEW" and "REQUIRESALARMS" definition. If you 
want to slim down the code feel free to use either of these by including



	#define REQUIRESNEW 

or 

	#define REQUIRESALARMS


at the top of DallasTemperature.h


## Credits

The OneWire code has been derived from
http://www.arduino.cc/playground/Learning/OneWire.
Miles Burton <miles@mnetcs.com> originally developed this library.
Tim Newsome <nuisance@casualhacker.net> added support for multiple sensors on
the same bus.
Guil Barros [gfbarros@bappos.com] added getTempByAddress (v3.5)
Rob Tillaart [rob.tillaart@gmail.com] added async modus (v3.7.0)


## Website


You can find the latest version of the library at
https://www.milesburton.com/Dallas_Temperature_Control_Library

# License

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

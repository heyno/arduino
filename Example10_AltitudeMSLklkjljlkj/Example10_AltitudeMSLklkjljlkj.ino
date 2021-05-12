/*
  Reading two altitudes - Mean Sea Level and Ellipsode
  By: Nathan Seidle
  SparkFun Electronics
  Date: January 3rd, 2019
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to query a u-blox module for its lat/long/altitude. 

  getAltitude() reports mm above ellipsode model of the globe. There are some
  instances where altitude above Mean Sea Level is better. This example shows how 
  to use getAltitudeMSL(). The difference varies but is ~20m.
  Ellipsoid model: https://www.esri.com/news/arcuser/0703/geoid1of3.html
  Difference between Ellipsoid Model and Mean Sea Level: https://eos-gnss.com/elevation-for-beginners/
  
  Leave NMEA parsing behind. Now you can simply ask the module for the datums you want!

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  ZED-F9P RTK2: https://www.sparkfun.com/products/15136
  NEO-M8P RTK: https://www.sparkfun.com/products/15005
  SAM-M8Q: https://www.sparkfun.com/products/15106

  Hardware Connections:
  Plug a Qwiic cable into the GNSS and a BlackBoard
  If you don't have a platform with a Qwiic connection use the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  Open the SerialUSB monitor at 115200 baud to see the output
*/

#include <Wire.h> //Needed for I2C to GNSS

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

long lastTime = 0; //Tracks the passing of 2000ms (2 seconds)

void setup()
{
  SerialUSB.begin(115200);
  while (!SerialUSB); //Wait for user to open terminal
  SerialUSB.println("SparkFun u-blox Example");

  Wire.begin();

  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    SerialUSB.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }
}

void loop()
{
  //Query module only every second. Doing it more often will just cause I2C traffic.
  //The module only responds when a new position is available
  if (millis() - lastTime > 1000)
  {
    lastTime = millis(); //Update the timer
    
    long latitude = myGNSS.getLatitude();
    SerialUSB.print(F("Lat: "));
    SerialUSB.print(latitude);

    long longitude = myGNSS.getLongitude();
    SerialUSB.print(F(" Long: "));
    SerialUSB.print(longitude);
    SerialUSB.print(F(" (degrees * 10^-7)"));

    long altitude = myGNSS.getAltitude();
    SerialUSB.print(F(" Alt: "));
    SerialUSB.print(altitude);
    SerialUSB.print(F(" (mm)"));

    long altitudeMSL = myGNSS.getAltitudeMSL();
    SerialUSB.print(F(" AltMSL: "));
    SerialUSB.print(altitudeMSL);
    SerialUSB.print(F(" (mm)"));

    SerialUSB.println();
  }
}

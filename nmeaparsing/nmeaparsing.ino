/*
  Read NMEA sentences over I2C using u-blox module SAM-M8Q, NEO-M8P, etc
  By: Nathan Seidle
  SparkFun Electronics
  Date: August 22nd, 2018
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example reads the NMEA characters over I2C and pipes them to MicroNMEA
  This example will output your current long/lat and satellites in view
 
  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  ZED-F9P RTK2: https://www.sparkfun.com/products/15136
  NEO-M8P RTK: https://www.sparkfun.com/products/15005
  SAM-M8Q: https://www.sparkfun.com/products/15106

  For more MicroNMEA info see https://github.com/stevemarple/MicroNMEA

  Hardware Connections:
  Plug a Qwiic cable into the GNSS and a BlackBoard
  If you don't have a platform with a Qwiic connection use the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  Open the serial monitor at 115200 baud to see the output
  Go outside! Wait ~25 seconds and you should see your lat/long
*/

#include <Wire.h> //Needed for I2C to GNSS

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

#include <MicroNMEA.h> //http://librarymanager/All#MicroNMEA
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

void setup()
{
  SerialUSB.begin(115200);
  SerialUSB.println("SparkFun u-blox Example");

  Wire.begin();

  if (myGNSS.begin() == false)
  {
    SerialUSB.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }
}

void loop()
{
  myGNSS.checkUblox(); //See if new data is available. Process bytes as they come in.

  if(nmea.isValid() == true)
  {
    long latitude_mdeg = nmea.getLatitude();
    long longitude_mdeg = nmea.getLongitude();

    SerialUSB.print("Latitude (deg): ");
    SerialUSB.println(latitude_mdeg / 1000000., 6);
    SerialUSB.print("Longitude (deg): ");
    SerialUSB.println(longitude_mdeg / 1000000., 6);
        SerialUSB.print("Nav. system: ");
    if (nmea.getNavSystem())
      SerialUSB.println(nmea.getNavSystem());
    else
      SerialUSB.println("none");

    SerialUSB.print("Num. satellites: ");
    SerialUSB.println(nmea.getNumSatellites());

    SerialUSB.print("HDOP: ");
    SerialUSB.println(nmea.getHDOP()/10., 1);

    SerialUSB.print("Date/time: ");
    SerialUSB.print(nmea.getYear());
    SerialUSB.print('-');
    SerialUSB.print(int(nmea.getMonth()));
    SerialUSB.print('-');
    SerialUSB.print(int(nmea.getDay()));
    SerialUSB.print('T');
    SerialUSB.print(int(nmea.getHour()));
    SerialUSB.print(':');
    SerialUSB.print(int(nmea.getMinute()));
    SerialUSB.print(':');
    SerialUSB.println(int(nmea.getSecond()));



    long alt;
    SerialUSB.print("Altitude (m): ");
    if (nmea.getAltitude(alt))
      SerialUSB.println(alt / 1000., 3);
    else
      SerialUSB.println("not available");

    SerialUSB.print("Speed: ");
    SerialUSB.println(nmea.getSpeed() / 1000., 3);
    SerialUSB.print("Course: ");
    SerialUSB.println(nmea.getCourse() / 1000., 3);
  }
  else
  {
    SerialUSB.print("No Fix - ");
    SerialUSB.print("Num. satellites: ");
    SerialUSB.println(nmea.getNumSatellites());
  }

  delay(250); //Don't pound too hard on the I2C bus
}

//This function gets called from the SparkFun u-blox Arduino Library
//As each NMEA character comes in you can specify what to do with it
//Useful for passing to other libraries like tinyGPS, MicroNMEA, or even
//a buffer, radio, etc.
void SFE_UBLOX_GNSS::processNMEA(char incoming)
{
  //Take the incoming char from the u-blox I2C port and pass it on to the MicroNMEA lib
  //for sentence cracking
  nmea.process(incoming);
}

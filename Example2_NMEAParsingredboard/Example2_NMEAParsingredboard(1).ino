#include <SFE_MicroOLED.h>

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
  Open the SerialUSB monitor at 115200 baud to see the output
  Go outside! Wait ~25 seconds and you should see your lat/long
*/

#include <Wire.h> //Needed for I2C to GNSS

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

#include <MicroNMEA.h> //http://librarymanager/All#MicroNMEA
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));
#define PIN_RESET 9

/*
// This is the old way of instantiating oled. You can still do it this way if you want to.
#define DC_JUMPER 1
MicroOLED oled(PIN_RESET, DC_JUMPER); // I2C declaration
*/

// From version v1.3, we can also instantiate oled like this (but only for I2C)
MicroOLED oled(PIN_RESET); // The TwoWire I2C port is passed to .begin instead
void setup()
{
    Wire.begin(); // <-- Change this to (e.g.) Qwiic.begin(); as required
  //Wire.setClock(400000); // Uncomment this line to increase the I2C bus speed to 400kHz


/*
  // This is the old way of initializing the OLED.
  // You can still do it this way if you want to - but only
  // if you instantiated oled using: MicroOLED oled(PIN_RESET, DC_JUMPER)
  oled.begin();    // Initialize the OLED
*/


  // This is the new way of initializing the OLED.
  // We can pass a different I2C address and TwoWire port
  oled.begin(0x3D, Wire);    // Initialize the OLED


/*
  // This is the new way of initializing the OLED.
  // We can pass a different I2C address and TwoWire port
  oled.begin(0x3C, Qwiic);    // Initialize the OLED
*/


  oled.clear(ALL); // Clear the display's internal memory
  oled.display();  // Display what's in the buffer (splashscreen)

  delay(1000); // Delay 1000 ms

  oled.clear(PAGE); // Clear the buffer.
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
    SerialUSB.print("Valid fix: ");
    SerialUSB.println(nmea.isValid() ? "yes" : "no");

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
    SerialUSB.print("Latitude (deg): ");
    SerialUSB.println(latitude_mdeg / 1000000., 6);
    SerialUSB.print("Longitude (deg): ");
    SerialUSB.println(longitude_mdeg / 1000000., 6);
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
      oled.setFontType(0);  // Set font to type 1
  oled.clear(PAGE);     // Clear the page
  oled.setCursor(0, 0); // Set cursor to top-left
  // Print can be used to print a string to the screen:
  oled.print(latitude_mdeg / 1000000.)
  oled.display(); // Refresh the display
  delay(1000);    // Delay a second and repeat
  oled.clear(PAGE);
  oled.setCursor(0, 0);
  oled.print("56789:;<=>?@ABCDEFGHI");
  oled.display();
  delay(1000);
  oled.clear(PAGE);
  oled.setCursor(0, 0);
  oled.print("JKLMNOPQRSTUVWXYZ[\\]^");
  oled.display();
  delay(1000);
  oled.clear(PAGE);
  oled.setCursor(0, 0);
  oled.print("_`abcdefghijklmnopqrs");
  oled.display();
  delay(1000);
  oled.clear(PAGE);
  oled.setCursor(0, 0);
  oled.print("tuvwxyz{|}~");
  oled.display();
  delay(1000);
  }
  else
  {
    SerialUSB.print("No Fix - ");
    SerialUSB.print("Num. satellites: ");
    SerialUSB.println(nmea.getNumSatellites());
     // Demonstrate font 1. 8x16. Let's use the print function
  // to display every character defined in this font.

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

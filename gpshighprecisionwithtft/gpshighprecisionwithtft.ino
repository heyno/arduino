/*
  Get the high precision geodetic solution for latitude and longitude using double
  By: Nathan Seidle
  Modified by: Paul Clark (PaulZC)
  SparkFun Electronics
  Date: April 17th, 2020
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to inspect the accuracy of the high-precision
  positional solution. Please see below for information about the units.

  ** This example will only work correctly on platforms which support 64-bit double **

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  ZED-F9P RTK2: https://www.sparkfun.com/products/15136
  NEO-M8P RTK: https://www.sparkfun.com/products/15005

  Hardware Connections:
  Plug a Qwiic cable into the GNSS and (e.g.) a Redboard Artemis https://www.sparkfun.com/products/15444
  or an Artemis Thing Plus https://www.sparkfun.com/products/15574
  If you don't have a platform with a Qwiic connection use the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  Open the serial monitor at 115200 baud to see the output
*/
#include "HyperDisplay_4DLCD-320240_4WSPI.h" // Click here to get the library: https://github.com/sparkfun/HyperDisplay_4DLCD-320240_ArduinoLibrary
                                           // Click here to get the mid-level library: http://librarymanager/All#SparkFun_HyperDisplay_ILI9341
                                           // Click here to get HyperDisplay (top level): http://librarymanager/All#SparkFun_HyperDisplay

#define SERIAL_PORT Serial    // Allows users to easily change target serial port (e.g. SAMD21's SerialUSB)

#define PWM_PIN PWM0             // Pin definitions
#define CS_PIN D0
#define DC_PIN D1
#define SPI_PORT SPI
#define SPI_SPEED 32000000    // Requests host uC to use the fastest possible SPI speed up to 32 MHz

LCD320240_4WSPI myTFT;

ILI9341_color_18_t defaultColor;
    wind_info_t wind1, wind2, wind3;  // Create several window objects
  ILI9341_color_18_t color1, color2, color3;

      double old_lat;
    double old_lon;
    double old_msl;
    double old_error;

#include <Wire.h> // Needed for I2C to GNSS

#define myWire Wire // This will work on the Redboard Artemis and the Artemis Thing Plus using Qwiic
//#define myWire Wire1 // Uncomment this line if you are using the extra SCL1/SDA1 pins (D17 and D16) on the Thing Plus

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to u-blox module.

void setup()
{
  old_lat = (double)0.0;
  old_lon = (double)0.0;
  old_msl = (double)0.0;
  old_error=(double)0.0;
  Serial.begin(115200);
  while (!Serial); //Wait for user to open terminal

  myWire.begin();

  //myGNSS.enableDebugging(Serial); // Uncomment this line to enable debug messages

  if (myGNSS.begin(myWire) == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1)
      ;
  }

  // Check that this platform supports 64-bit (8 byte) double
  if (sizeof(double) < 8)
  {
    Serial.println(F("Warning! Your platform does not support 64-bit double."));
    Serial.println(F("The latitude and longitude will be inaccurate."));
  }
  myTFT.begin(DC_PIN, CS_PIN, PWM_PIN, SPI_PORT, SPI_SPEED);  // This is a non-hyperdisplay function, but it is required to make the display work
  myTFT.setInterfacePixelFormat(ILI9341_PXLFMT_18);
  myTFT.clearDisplay();                                       // clearDisplay is also not pat of hyperdisplay, but we will use it here for simplicity

  defaultColor = myTFT.rgbTo18b( 255, 255, 255 );
  myTFT.setCurrentWindowColorSequence((color_t)&defaultColor);


  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)

  //myGNSS.setNavigationFrequency(20); //Set output to 20 times a second

  byte rate = myGNSS.getNavigationFrequency(); //Get the update rate of this module
  Serial.print("Current update rate: ");
  Serial.println(rate);

  //myGNSS.saveConfiguration(); //Save the current settings to flash and BBR
    bool response = true;

  //Read the settings from RAM (what the module is running right now, not BBR, Flash, or default)
  uint8_t currentUART1Setting_ubx = myGNSS.getVal8(UBLOX_CFG_UART1INPROT_UBX);
  uint8_t currentUART1Setting_nmea = myGNSS.getVal8(UBLOX_CFG_UART1INPROT_NMEA);
  uint8_t currentUART1Setting_rtcm3 = myGNSS.getVal8(UBLOX_CFG_UART1INPROT_RTCM3X);

  Serial.print("currentUART1Setting_ubx: ");
  Serial.println(currentUART1Setting_ubx);
  Serial.print("currentUART1Setting_nmea: ");
  Serial.println(currentUART1Setting_nmea);
  Serial.print("currentUART1Setting_rtcm3: ");
  Serial.println(currentUART1Setting_rtcm3);

  //Check if NMEA and RTCM are enabled for UART1
  if (currentUART1Setting_ubx == 0 || currentUART1Setting_nmea == 0)
  {
    Serial.println("Updating UART1 configuration");

    //setVal sets the values for RAM, BBR, and Flash automatically so no .saveConfiguration() is needed
    response &= myGNSS.setVal8(UBLOX_CFG_UART1INPROT_UBX, 1);    //Enable UBX on UART1 Input
    response &= myGNSS.setVal8(UBLOX_CFG_UART1INPROT_NMEA, 1);   //Enable NMEA on UART1 Input
    response &= myGNSS.setVal8(UBLOX_CFG_UART1INPROT_RTCM3X, 0); //Disable RTCM on UART1 Input

    if (response == false)
      Serial.println("SetVal failed");
    else
      Serial.println("SetVal succeeded");
  }
  else
    Serial.println("No port change needed");

  //Change speed of UART2
  uint32_t currentUART2Baud = myGNSS.getVal32(UBLOX_CFG_UART2_BAUDRATE);
  Serial.print("currentUART2Baud: ");
  Serial.println(currentUART2Baud);

  if (currentUART2Baud != 115200)
  {
    response &= myGNSS.setVal32(UBLOX_CFG_UART2_BAUDRATE, 115200);
    if (response == false)
      Serial.println("SetVal failed");
    else
      Serial.println("SetVal succeeded");
  }
  else
    Serial.println("No baud change needed");

  Serial.println("Done");
    //Query module only every second.
  //The module only responds when a new position is available.


  // Initialize the windows to defualt settings (this is a pretty important step unless you are extra careful to manually initialize each and every paramter)
  myTFT.setWindowDefaults(&wind1);
  myTFT.setWindowDefaults(&wind2);
  myTFT.setWindowDefaults(&wind3);

  color1.r = 0xFF;    // Set the colors to red, green, and blue respectively
  color1.g = 0x00;
  color1.b = 0x00;

  color2.r = 0x00;
  color2.g = 0xFF;
  color2.b = 0x00;

  color3.r = 0x00;
  color3.g = 0x00;
  color3.b = 0xFF;

  // Now we will set up the boundaries of the windows, the cursor locations, and their default colors
  wind1.xMin = 0;
  wind1.yMin = 0;
  wind1.xMax = 118;
  wind1.yMax = 118;
  wind1.cursorX = 1;
  wind1.cursorY = 1;
  wind1.xReset = 1;
  wind1.yReset = 1;
  wind1.currentSequenceData = (color_t)&color1;
  wind1.currentColorCycleLength = 1;
  wind1.currentColorOffset = 0;

  wind2.xMin = 121;
  wind2.yMin = 0;
  wind2.xMax = 239;
  wind2.yMax = 118;
  wind2.cursorX = 15;
  wind2.cursorY = 15;
  wind2.xReset = 15;
  wind2.yReset = 15;
  wind2.currentSequenceData = (color_t)&color2;
  wind2.currentColorCycleLength = 1;
  wind2.currentColorOffset = 0;

  wind3.xMin = 0;
  wind3.yMin = 121;
  wind3.xMax = 239;
  wind3.yMax = 319;
  wind3.cursorX = 1;
  wind3.cursorY = 1;
  wind3.xReset = 10;                              // This extra line will give a hanging indent effect for window 3
  wind3.yReset = 1;
  wind3.currentSequenceData = (color_t)&color3;
  wind3.currentColorCycleLength = 1;
  wind3.currentColorOffset = 0;

  // All hyperdisplay drawing functions are applied to the current window, and the coordinates are with respect to the window.
  // To demonstrate this we will use the same exact drawing function to draw in each of the three windows, each with to unique effect
  wind_info_t* windowPointers[] = {&wind1, &wind2, &wind3};
  for(uint8_t indi = 0; indi < 3; indi++){
    myTFT.pCurrentWindow = windowPointers[indi];  // Set the current window
    myTFT.line(1,1, 238,78);                      // Use the window's default color to draw a line on the window - notice that it is truncated beyond the window area
  }
  delay(5000);

  // The 'fillWindow' function can be used with or without a specified color
  for(uint8_t indi = 0; indi < 3; indi++){
    myTFT.pCurrentWindow = windowPointers[indi];  // Set the current window
    myTFT.fillWindow();                           // Fill the window with the window's color
  }
  delay(5000);

  // And lastly, the printing functions are impacted by windows.
  for(uint8_t indi = 0; indi < 3; indi++){
    myTFT.pCurrentWindow = windowPointers[indi];                  // Set the current window
    myTFT.setCurrentWindowColorSequence((color_t)&defaultColor);  // Change each window's color to the white value we defined at the beginning
//    for(int i0=0; i0<20; i0++)
//    {
//      myTFT.println("You see, each window handles printed text on its own!");
//    }
//
//    myTFT.print("Done!");
   }
}

void loop()
{
  
  
  if (millis() - lastTime > 1000)
  {
    lastTime = millis(); //Update the timer

    // getHighResLatitude: returns the latitude from HPPOSLLH as an int32_t in degrees * 10^-7
    // getHighResLatitudeHp: returns the high resolution component of latitude from HPPOSLLH as an int8_t in degrees * 10^-9
    // getHighResLongitude: returns the longitude from HPPOSLLH as an int32_t in degrees * 10^-7
    // getHighResLongitudeHp: returns the high resolution component of longitude from HPPOSLLH as an int8_t in degrees * 10^-9
    // getElipsoid: returns the height above ellipsoid as an int32_t in mm
    // getElipsoidHp: returns the high resolution component of the height above ellipsoid as an int8_t in mm * 10^-1
    // getMeanSeaLevel: returns the height above mean sea level as an int32_t in mm
    // getMeanSeaLevelHp: returns the high resolution component of the height above mean sea level as an int8_t in mm * 10^-1
    // getHorizontalAccuracy: returns the horizontal accuracy estimate from HPPOSLLH as an uint32_t in mm * 10^-1

    // First, let's collect the position data
    int32_t latitude = myGNSS.getHighResLatitude();
    int8_t latitudeHp = myGNSS.getHighResLatitudeHp();
    int32_t longitude = myGNSS.getHighResLongitude();
    int8_t longitudeHp = myGNSS.getHighResLongitudeHp();
    int32_t ellipsoid = myGNSS.getElipsoid();
    int8_t ellipsoidHp = myGNSS.getElipsoidHp();
    int32_t msl = myGNSS.getMeanSeaLevel();
    int8_t mslHp = myGNSS.getMeanSeaLevelHp();
    uint32_t accuracy = myGNSS.getHorizontalAccuracy();

    // Defines storage for the lat and lon as double
    double d_lat; // latitude
    double d_lon; // longitude


    // Assemble the high precision latitude and longitude
    d_lat = ((double)latitude) / 10000000.0; // Convert latitude from degrees * 10^-7 to degrees
    d_lat += ((double)latitudeHp) / 1000000000.0; // Now add the high resolution component (degrees * 10^-9 )
    d_lon = ((double)longitude) / 10000000.0; // Convert longitude from degrees * 10^-7 to degrees
    d_lon += ((double)longitudeHp) / 1000000000.0; // Now add the high resolution component (degrees * 10^-9 )

   // Print the lat and lon
    Serial.print("Lat (deg): ");
    Serial.print(d_lat, 9);
    Serial.print(", Lon (deg): ");
    Serial.print(d_lon, 9);

    // Now define float storage for the heights and accuracy
    float f_ellipsoid;
    float f_msl;
    float f_accuracy;

    // Calculate the height above ellipsoid in mm * 10^-1
    f_ellipsoid = (ellipsoid * 10) + ellipsoidHp;
    // Now convert to m
    f_ellipsoid = f_ellipsoid / 10000.0; // Convert from mm * 10^-1 to m

    // Calculate the height above mean sea level in mm * 10^-1
    f_msl = (msl * 10) + mslHp;
    // Now convert to m
    f_msl = f_msl / 10000.0; // Convert from mm * 10^-1 to m

    // Convert the horizontal accuracy (mm * 10^-1) to a float
    f_accuracy = accuracy;
    // Now convert to m
    f_accuracy = f_accuracy / 10000.0; // Convert from mm * 10^-1 to m

    // Finally, do the printing
    Serial.print(", Ellipsoid (m): ");
    Serial.print(f_ellipsoid, 4); // Print the ellipsoid with 4 decimal places

    Serial.print(", Mean Sea Level (m): ");
    Serial.print(f_msl, 4); // Print the mean sea level with 4 decimal places

    Serial.print(", Accuracy (m): ");
    Serial.println(f_accuracy, 4); // Print the accuracy with 4 decimal places
      wind_info_t* windowPointers[] = {&wind1, &wind2, &wind3};
    myTFT.pCurrentWindow = windowPointers[0]; 
      myTFT.setWindowDefaults(&wind1);
    myTFT.fillWindow(); 
    myTFT.setCurrentWindowColorSequence((color_t)&color1);
    myTFT.setTextCursor(0,0);
    if (old_lat != 0.0)
    {
      myTFT.print(old_lat,9); 
    }   
    myTFT.setCurrentWindowColorSequence((color_t)&defaultColor);  // Change each window's color to the white value we defined at the beginning
    myTFT.setTextCursor(0,0);
    myTFT.print(d_lat,9);
    old_lat = (double) d_lat;
    myTFT.setTextCursor(0,10);
    myTFT.setCurrentWindowColorSequence((color_t)&color1);
    if (old_lon != 0.0)
    {
      myTFT.print(old_lon,9);
    }
       myTFT.setTextCursor(0,10);
       myTFT.setCurrentWindowColorSequence((color_t)&defaultColor);  // Change each window's color to the white value we defined at the beginning

 
    myTFT.print(d_lon,9);
    old_lon = (double)d_lon;
        myTFT.setTextCursor(0,20);
    myTFT.setCurrentWindowColorSequence((color_t)&color1);
    if (old_msl != 0.0)
    {
      myTFT.print(old_msl,4);
    }

       myTFT.setTextCursor(0,20);
       myTFT.setCurrentWindowColorSequence((color_t)&defaultColor);  // Change each window's color to the white value we defined at the beginning

 
    myTFT.print(f_msl,4);
    old_msl = (double)f_msl;
           myTFT.setTextCursor(0,30);
    myTFT.setCurrentWindowColorSequence((color_t)&color1);
    if (old_error != 0.0)
    {
      myTFT.print(old_error,4);
    }

       myTFT.setTextCursor(0,30);
       myTFT.setCurrentWindowColorSequence((color_t)&defaultColor);  // Change each window's color to the white value we defined at the beginning

 
    myTFT.print(f_accuracy,4);
    old_error = (double)f_accuracy;
  
  }
  
}

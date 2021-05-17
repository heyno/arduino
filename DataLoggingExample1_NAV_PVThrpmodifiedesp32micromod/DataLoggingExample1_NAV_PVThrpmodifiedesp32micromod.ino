/*
  Configuring the GNSS to automatically send NAV PVT reports over I2C and log them to file on SD card
  By: Paul Clark
  SparkFun Electronics
  Date: December 30th, 2020
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to configure the u-blox GNSS to send NAV PVT reports automatically
  and log the data to SD card in UBX format.

  This code is intended to be run on the MicroMod Data Logging Carrier Board using the Artemis Processor
  but can be adapted by changing the chip select pin and SPI definitions:
  https://www.sparkfun.com/products/16829
  https://www.sparkfun.com/products/16401

  Hardware Connections:
  Please see: https://learn.sparkfun.com/tutorials/micromod-data-logging-carrier-board-hookup-guide
  Insert the Artemis Processor into the MicroMod Data Logging Carrier Board and secure with the screw.
  Connect your GNSS breakout to the Carrier Board using a Qwiic cable.
  Connect an antenna to your GNSS board if required.
  Insert a formatted micro-SD card into the socket on the Carrier Board.
  Connect the Carrier Board to your computer using a USB-C cable.
  Ensure you have the SparkFun Apollo3 boards installed: http://boardsmanager/All#SparkFun_Apollo3
  This code has been tested using version 1.2.1 of the Apollo3 boards on Arduino IDE 1.8.13.
  Select "SparkFun Artemis MicroMod" as the board type.
  Press upload to upload the code onto the Artemis.
  Open the Serial Monitor at 115200 baud to see the output.

  To minimise I2C bus errors, it is a good idea to open the I2C pull-up split pad links on
  both the MicroMod Data Logging Carrier Board and the u-blox module breakout.

  Data is logged in u-blox UBX format. Please see the u-blox protocol specification for more details.
  You can replay and analyze the data using u-center:
  https://www.u-blox.com/en/product/u-center

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  ZED-F9P RTK2: https://www.sparkfun.com/products/15136
  NEO-M8P RTK: https://www.sparkfun.com/products/15005
  SAM-M8Q: https://www.sparkfun.com/products/15106

*/

//#include <SPI.h>
//#include <SD.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include<SparkFun_MicroMod_Button.h>
#include <Wire.h> //Needed for I2C to GNSS

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;
#include "HyperDisplay_4DLCD-320240_4WSPI.h" // Click here to get the library: https://github.com/sparkfun/HyperDisplay_4DLCD-320240_ArduinoLibrary
                                           // Click here to get the mid-level library: http://librarymanager/All#SparkFun_HyperDisplay_ILI9341
                                           // Click here to get HyperDisplay (top level): http://librarymanager/All#SparkFun_HyperDisplay
                                           // This is a further comment

#define SERIAL_PORT Serial    // Allows users to easily change target serial port (e.g. SAMD21's SerialUSB)

#define PWM_PIN PWM0             // Pin definitions
#define CS_PIN D0
#define DC_PIN D1
#define SPI_PORT SPI
#define SPI_SPEED 32000000    // Requests host uC to use the fastest possible SPI speed up to 32 MHz
MicroModButton button;
LCD320240_4WSPI myTFT;

ILI9341_color_18_t defaultColor;
    wind_info_t wind1, wind2, wind3;  // Create several window objects
  ILI9341_color_18_t color1, color2, color3;

    long old_lat;
    long old_lon;
    long old_msl;
    long old_error;
File myFile; //File that all GNSS data is written to

#define sdChipSelect 55 //Primary SPI Chip Select is CS for the MicroMod Artemis Processor. Adjust for your processor if necessary.

#define packetLength 100 // NAV PVT is 92 + 8 bytes in length (including the sync chars, class, id, length and checksum bytes)

// Callback: printPVTdata will be called when new NAV PVT data arrives
// See u-blox_structs.h for the full definition of UBX_NAV_PVT_data_t
//         _____  You can use any name you like for the callback. Use the same name when you call setAutoPVTcallback
//        /                  _____  This _must_ be UBX_NAV_PVT_data_t
//        |                 /               _____ You can use any name you like for the struct
//        |                 |              /
//        |                 |              |
void printPVTdata(UBX_NAV_PVT_data_t ubxDataStruct)
{
    Serial.println();

    Serial.print(F("Time: ")); // Print the time
    uint8_t hms = ubxDataStruct.hour; // Print the hours
    if (hms < 10) Serial.print(F("0")); // Print a leading zero if required
    Serial.print(hms);
    Serial.print(F(":"));
    hms = ubxDataStruct.min; // Print the minutes
    if (hms < 10) Serial.print(F("0")); // Print a leading zero if required
    Serial.print(hms);
    Serial.print(F(":"));
    hms = ubxDataStruct.sec; // Print the seconds
    if (hms < 10) Serial.print(F("0")); // Print a leading zero if required
    Serial.print(hms);
    Serial.print(F("."));
    unsigned long millisecs = ubxDataStruct.iTOW % 1000; // Print the milliseconds
    if (millisecs < 100) Serial.print(F("0")); // Print the trailing zeros correctly
    if (millisecs < 10) Serial.print(F("0"));
    Serial.print(millisecs);

    long latitude = ubxDataStruct.lat; // Print the latitude
    Serial.print(F(" Lat: "));
    Serial.print(latitude);

    long longitude = ubxDataStruct.lon; // Print the longitude
    Serial.print(F(" Long: "));
    Serial.print(longitude);
    Serial.print(F(" (degrees * 10^-7)"));

    long altitude = ubxDataStruct.hMSL; // Print the height above mean sea level
    Serial.print(F(" Height above MSL: "));
    Serial.print(altitude);
    Serial.println(F(" (mm)"));
    wind_info_t* windowPointers[] = {&wind1, &wind2, &wind3};
    myTFT.pCurrentWindow = windowPointers[0]; 
      myTFT.setWindowDefaults(&wind1);
//    myTFT.fillWindow(); 
    myTFT.setCurrentWindowColorSequence((color_t)&color1);
    myTFT.setTextCursor(0,0);
    if (old_lat != 0.0)
    {
      myTFT.print(old_lat); 
    }   
    myTFT.setCurrentWindowColorSequence((color_t)&defaultColor);  // Change each window's color to the white value we defined at the beginning
    myTFT.setTextCursor(0,0);
    myTFT.print(latitude);
    old_lat = latitude;
    myTFT.setTextCursor(0,10);
    myTFT.setCurrentWindowColorSequence((color_t)&color1);
    if (old_lon != 0.0)
    {
      myTFT.print(old_lon);
    }
       myTFT.setTextCursor(0,10);
       myTFT.setCurrentWindowColorSequence((color_t)&defaultColor);  // Change each window's color to the white value we defined at the beginning

 
    myTFT.print(longitude);
    old_lon = longitude;
        myTFT.setTextCursor(0,20);
    myTFT.setCurrentWindowColorSequence((color_t)&color1);
    if (old_msl != 0.0)
    {
      myTFT.print(old_msl);
    }

       myTFT.setTextCursor(0,20);
       myTFT.setCurrentWindowColorSequence((color_t)&defaultColor);  // Change each window's color to the white value we defined at the beginning

 
    myTFT.print(altitude);
    old_msl = altitude;
//           myTFT.setTextCursor(0,30);
//    myTFT.setCurrentWindowColorSequence((color_t)&color1);
//    if (old_error != 0.0)
//    {
//      myTFT.print(old_error,4);
//    }
//
//       myTFT.setTextCursor(0,30);
//       myTFT.setCurrentWindowColorSequence((color_t)&defaultColor);  // Change each window's color to the white value we defined at the beginning
//
// 
//    myTFT.print(f_accuracy,4);
//    old_error = (double)f_accuracy;
  
}

void setup()
{
  Serial.begin(115200);
  while (!Serial); //Wait for user to open terminal
  Serial.println("SparkFun u-blox Example");

  Wire.begin(); // Start I2C communication with the GNSS
  



  
  if(!button.begin()) //Connect to the buttons 
  {
    Serial.println("Buttons not found");
    while(1);
  }
  Serial.println("Buttons connected!");
    myTFT.begin(DC_PIN, CS_PIN, PWM_PIN, SPI_PORT, SPI_SPEED);  // This is a non-hyperdisplay function, but it is required to make the display work
  myTFT.setInterfacePixelFormat(ILI9341_PXLFMT_18);
  myTFT.clearDisplay();                                       // clearDisplay is also not pat of hyperdisplay, but we will use it here for simplicity

  defaultColor = myTFT.rgbTo18b( 255, 255, 255 );
  myTFT.setCurrentWindowColorSequence((color_t)&defaultColor);

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

#if defined(AM_PART_APOLLO3)
  Wire.setPullups(0); // On the Artemis, we can disable the internal I2C pull-ups too to help reduce bus errors
#endif
  int button_pressed = 0;
  while (button_pressed==0)
  {
    if(button.getPressedInterrupt())  //Check to see if a button has been pressed
    {
      uint8_t pressed = button.getPressed(); //Read which button has been pressed
      
      if(pressed & 0x01)
      {
        Serial.println("Button A pressed!");
      }
      if(pressed & 0x02)
      {
        Serial.println("Button B pressed!");
      }
      if(pressed & 0x04)
      {
        Serial.println("Button UP pressed!");
      }
      if(pressed & 0x08)
      {
        Serial.println("Button DOWN pressed!");
      }
      if(pressed & 0x10)
      {
        Serial.println("Button LEFT pressed!");
      }
      if(pressed & 0x20)
      {
        Serial.println("Button RIGHT pressed!");
      }
      if(pressed & 0x40)
      {
        Serial.println("Button CENTER pressed!");
      }
      delay(100);
    }
  
    if(button.getClickedInterrupt())  //Check to see if a button has been released
    {
      uint8_t clicked = button.getClicked(); //Read which button has been released
      
      if(clicked & 0x01)
      {
        Serial.println("Button A released!");
        button_pressed = 1;
      }
      if(clicked & 0x02)
      {
        Serial.println("Button B released!");
      }
      if(clicked & 0x04)
      {
        Serial.println("Button UP released!");
      }
      if(clicked & 0x08)
      {
        Serial.println("Button DOWN released!");
      }
      if(clicked & 0x10)
      {
        Serial.println("Button LEFT released!");
      }
      if(clicked & 0x20)
      {
        Serial.println("Button RIGHT released!");
      }
      if(clicked & 0x40)
      {
        Serial.println("Button CENTER released!");
      }
    }
  }
//  while (Serial.available()) // Make sure the Serial buffer is empty
//  {
//    Serial.read();
//  }
//
//  Serial.println(F("Press any key to start logging."));
//
//  while (!Serial.available()) // Wait for the user to press a key
//  {
//    ; // Do nothing
//  }

  delay(100); // Wait, just in case multiple characters were sent

  while (Serial.available()) // Empty the Serial buffer
  {
    Serial.read();
  }

  Serial.println("Initializing SD card...");

  // See if the card is present and can be initialized:
  if (!SD.begin())
  {
    Serial.println("Card failed, or not present. Freezing...");
    // don't do anything more:
    while (1);
  }
      uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
Serial.println("SD card initialized.");

  // Create or open a file called "NAV_PVT.ubx" on the SD card.
  // If the file already exists, the new data is appended to the end of the file.
  myFile = SD.open("/NAV4_PVT.ubx", FILE_WRITE);
  if(!myFile)
  {
    Serial.println(F("Failed to create UBX data file! Freezing..."));
    while (1);
  }

  //myGNSS.enableDebugging(); // Uncomment this line to enable helpful GNSS debug messages on Serial

  // NAV PVT messages are 100 bytes long.
  // In this example, the data will arrive no faster than one message per second.
  // So, setting the file buffer size to 301 bytes should be more than adequate.
  // I.e. room for three messages plus an empty tail byte.
  myGNSS.setFileBufferSize(301); // setFileBufferSize must be called _before_ .begin

  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing..."));
    while (1);
  }

  // Uncomment the next line if you want to reset your module back to the default settings with 1Hz navigation rate
  // (This will also disable any "auto" messages that were enabled and saved by other examples and reduce the load on the I2C bus)
  //myGNSS.factoryDefault(); delay(5000);

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

  myGNSS.setNavigationFrequency(1); //Produce one navigation solution per second

  myGNSS.setAutoPVTcallback(&printPVTdata); // Enable automatic NAV PVT messages with callback to printPVTdata

  myGNSS.logNAVPVT(); // Enable NAV PVT data logging

  Serial.println(F("Press any key to stop logging."));
}

void loop()
{
  myGNSS.checkUblox(); // Check for the arrival of new data and process it.
  myGNSS.checkCallbacks(); // Check if any callbacks are waiting to be processed.

  if (myGNSS.fileBufferAvailable() >= packetLength) // Check to see if a new packetLength-byte NAV PVT message has been stored
  {
    uint8_t myBuffer[packetLength]; // Create our own buffer to hold the data while we write it to SD card

    myGNSS.extractFileBufferData((uint8_t *)&myBuffer, packetLength); // Extract exactly packetLength bytes from the UBX file buffer and put them into myBuffer

    myFile.write(myBuffer, packetLength); // Write exactly packetLength bytes from myBuffer to the ubxDataFile on the SD card

    //printBuffer(myBuffer); // Uncomment this line to print the data as Hexadecimal bytes
  }
    if(button.getPressedInterrupt())  //Check to see if a button has been pressed
    {
      uint8_t pressed = button.getPressed(); //Read which button has been pressed
      
      if(pressed & 0x01)
      {
        Serial.println("Button A pressed!");
      }
      if(pressed & 0x02)
      {
        Serial.println("Button B pressed!");
      }
      if(pressed & 0x04)
      {
        Serial.println("Button UP pressed!");
      }
      if(pressed & 0x08)
      {
        Serial.println("Button DOWN pressed!");
      }
      if(pressed & 0x10)
      {
        Serial.println("Button LEFT pressed!");
      }
      if(pressed & 0x20)
      {
        Serial.println("Button RIGHT pressed!");
      }
      if(pressed & 0x40)
      {
        Serial.println("Button CENTER pressed!");
      }
      delay(100);
    }
  
    if(button.getClickedInterrupt())  //Check to see if a button has been released
    {
      uint8_t clicked = button.getClicked(); //Read which button has been released
      
      if(clicked & 0x01)
      {
        Serial.println("Button A released!");

      }
      if(clicked & 0x02)
      {
        Serial.println("Button B released!");
            myFile.close(); // Close the data file
    Serial.println(F("Logging stopped. Freezing..."));
    while(1); // Do nothing more
      }
      if(clicked & 0x04)
      {
        Serial.println("Button UP released!");
      }
      if(clicked & 0x08)
      {
        Serial.println("Button DOWN released!");
      }
      if(clicked & 0x10)
      {
        Serial.println("Button LEFT released!");
      }
      if(clicked & 0x20)
      {
        Serial.println("Button RIGHT released!");
      }
      if(clicked & 0x40)
      {
        Serial.println("Button CENTER released!");
      }
    }
  if (Serial.available()) // Check if the user wants to stop logging
  {
    myFile.close(); // Close the data file
    Serial.println(F("Logging stopped. Freezing..."));
    while(1); // Do nothing more
  }

  Serial.print(".");
  delay(50);
}

// Print the buffer contents as Hexadecimal bytes
// You should see:
// SYNC CHAR 1: 0xB5
// SYNC CHAR 2: 0x62
// CLASS: 0x01 for NAV
// ID: 0x07 for PVT
// LENGTH: 2-bytes Little Endian (0x5C00 = 92 bytes for NAV PVT)
// PAYLOAD: LENGTH bytes
// CHECKSUM_A
// CHECKSUM_B
// Please see the u-blox protocol specification for more details
void printBuffer(uint8_t *ptr)
{
  for (int i = 0; i < packetLength; i++)
  {
    if (ptr[i] < 16) Serial.print("0"); // Print a leading zero if required
    Serial.print(ptr[i], HEX); // Print the byte as Hexadecimal
    Serial.print(" ");
  }
  Serial.println();
}

#include <SparkFun_TMP117.h>
#include <SparkFun_TMP117_Registers.h>

//******************************************************************************/
#include <Wire.h>
#include <SparkFunBME280.h> //Click here to get the library: http://librarymanager/All#SparkFun_BME280
#include <SparkFunCCS811.h> //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include "SparkFun_Qwiic_OpenLog_Arduino_Library.h"
#include <SparkFun_RV8803.h>
#include <SFE_MicroOLED.h> //Click here to get the library: http://librarymanager/All#SparkFun_Micro_OLED
// Added a comment to see if git picks it up

#define PIN_RESET 9
#define DC_JUMPER 1
MicroOLED oled(PIN_RESET, DC_JUMPER); // I2C declaration

RV8803 rtc;
OpenLog myLog; //Create instance
#define CCS811_ADDR 0x5B //Default I2C Address
//#define CCS811_ADDR 0x5A //Alternate I2C Address

#define PIN_NOT_WAKE 5

//Global sensor objects
CCS811 myCCS811(CCS811_ADDR);
BME280 myBME280;
TMP117 sensor; // Initalize sensor

void setup()
{
  SerialUSB.begin(115200);
//  while (!SerialUSB) {
//    ; // wait for SerialUSB port to connect. Needed for native USB port only
//  }
  SerialUSB.println();
  SerialUSB.println("Apply BME280 data to CCS811 for compensation.");

  Wire.begin();
//    Wire.setClock(400000);   // Set clock speed to be the fastest for better communication (fast mode)


    oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  oled.display();  // Display what's in the buffer (splashscreen)

  delay(1000); // Delay 1000 ms

  oled.clear(PAGE); // Clear the buffer.
  myLog.begin(); //Open connection to OpenLog (no pun intended)
  //This begins the CCS811 sensor and prints error status of .beginWithStatus()
  CCS811Core::CCS811_Status_e returnCode = myCCS811.beginWithStatus();
  SerialUSB.print("CCS811 begin exited with: ");
  SerialUSB.println(myCCS811.statusString(returnCode));

  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x77;

  //Initialize BME280
  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x77;
  myBME280.settings.runMode = 3; //Normal mode
  myBME280.settings.tStandby = 0;
  myBME280.settings.filter = 4;
  myBME280.settings.tempOverSample = 5;
  myBME280.settings.pressOverSample = 5;
  myBME280.settings.humidOverSample = 5;
  if (sensor.begin() == true) // Function to check if the sensor will correctly self-identify with the proper Device ID/Address
  {
    SerialUSB.println("Begin tmp117");
  }
  else
  {
    SerialUSB.println("Device failed to setup- Freezing code.");
    while (1); // Runs forever
  }
  //Calling .begin() causes the settings to be loaded
  delay(10); //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  myBME280.begin();
    if (rtc.begin() == false) {
    SerialUSB.println("Something went wrong, check wiring");
  }
  else
  {
    SerialUSB.println("RTC online!");
  }
  
}
// Center and print a small title
// This function is quick and dirty. Only works for titles one
// line long.
void printTitle(String title, int font)
{
  int middleX = oled.getLCDWidth() / 2;
  int middleY = oled.getLCDHeight() / 2;

  oled.clear(PAGE);
  oled.setFontType(font);
  // Try to set the cursor in the middle of the screen
  oled.setCursor(middleX - (oled.getFontWidth() * (title.length() / 2)),
                 middleY - (oled.getFontHeight() / 2));
  // Print the title:
  oled.print(title);
  oled.display();
  delay(1500);
  oled.clear(PAGE);
}
void textExamples()
{


  // Demonstrate font 0. 5x8 font
  oled.clear(PAGE);     // Clear the screen
  oled.setFontType(0);  // Set font to type 0
  oled.setCursor(0, 0); // Set cursor to top-left
  

  // Demonstrate font 1. 8x16. Let's use the print function
  // to display every character defined in this font.
  oled.setFontType(2);  // Set font to type 1
  oled.clear(PAGE);     // Clear the page
  oled.setCursor(0, 0); // Set cursor to top-left

    // Demonstrate font 2. 10x16. Only numbers and '.' are defined.
  // This font looks like 7-segment displays.
  // Lets use this big-ish font to display readings from the
  // analog pins.
  String currentTime = rtc.stringTime(); //Get the time
    oled.clear(PAGE);           // Clear the display
    oled.setCursor(0, 0);       // Set cursor to top-left
    oled.setFontType(0);        // Smallest font
    oled.print("C: ");         // Print "A0"
    oled.setFontType(0);        // 7-segment font
    oled.print(myCCS811.getCO2()); // Print a0 reading
    oled.setCursor(0, 12);      // Set cursor to top-middle-left
    oled.setFontType(0);        // Repeat
    oled.print("V: ");
    oled.setFontType(0);
    oled.print(myCCS811.getTVOC());
    oled.setCursor(0, 24);
    oled.setFontType(0);
    oled.print("T: ");
    oled.setFontType(0);
    oled.print(myBME280.readTempC());
    oled.setCursor(0,36);
    oled.print(currentTime);
    oled.display();
    delay(100);
}

//---------------------------------------------------------------
void loop()
{
  //Check to see if data is available
  if (myCCS811.dataAvailable())
  {
    //Calling this function updates the global tVOC and eCO2 variables
    myCCS811.readAlgorithmResults();
    //printInfoSerialUSB fetches the values of tVOC and eCO2
    printInfoSerialUSB();

    float BMEtempC = myBME280.readTempC()-2.93; // Compensate for temp delta
    float BMEhumid =  myBME280.readFloatHumidity();

    SerialUSB.print("Applying new values (deg C, %): ");
    SerialUSB.print(BMEtempC);
    SerialUSB.print(",");
    SerialUSB.println(BMEhumid);
    SerialUSB.println();


    //This sends the temperature data to the CCS811
    myCCS811.setEnvironmentalData(BMEhumid, BMEtempC);
  }
  else if (myCCS811.checkForStatusError())
  {
    //If the CCS811 found an internal error, print it.
    printSensorError();
  }

  delay(2000); //Wait for next reading
}

//---------------------------------------------------------------
void printInfoSerialUSB()
{    float tempC = sensor.readTempC();
    float tempF = sensor.readTempF();
    if (sensor.dataReady() == true) // Function to make sure that there is data ready to be printed, only prints temperature values when data is ready
 {

    // Print temperature in °C and °F
    SerialUSB.println(); // Create a white space for easier viewing
    SerialUSB.print("Temperature in Celsius: ");
    SerialUSB.println(tempC);
    SerialUSB.print("Temperature in Fahrenheit: ");
    SerialUSB.println(tempF);


    
  }
  //getCO2() gets the previously read data from the library
  SerialUSB.println("CCS811 data:");
  SerialUSB.print(" CO2 concentration : ");
  SerialUSB.print(myCCS811.getCO2());
  SerialUSB.println(" ppm");

  //getTVOC() gets the previously read data from the library
  SerialUSB.print(" TVOC concentration : ");
  SerialUSB.print(myCCS811.getTVOC());
  SerialUSB.println(" ppb");

  SerialUSB.println("BME280 data:");
  SerialUSB.print(" Temperature: ");
  SerialUSB.print(myBME280.readTempC(), 2);
  SerialUSB.println(" degrees C");

  SerialUSB.print(" Temperature: ");
  SerialUSB.print(myBME280.readTempF(), 2);
  SerialUSB.println(" degrees F");

  SerialUSB.print(" Pressure: ");
  SerialUSB.print(myBME280.readFloatPressure(), 2);
  SerialUSB.println(" Pa");

  SerialUSB.print(" Pressure: ");
  SerialUSB.print((myBME280.readFloatPressure() * 0.0002953), 2);
  SerialUSB.println(" InHg");

  SerialUSB.print(" Altitude: ");
  SerialUSB.print(myBME280.readFloatAltitudeMeters(), 2);
  SerialUSB.println("m");

  SerialUSB.print(" Altitude: ");
  SerialUSB.print(myBME280.readFloatAltitudeFeet(), 2);
  SerialUSB.println("ft");

  SerialUSB.print(" %RH: ");
  SerialUSB.print(myBME280.readFloatHumidity(), 2);
  SerialUSB.println(" %");

  SerialUSB.println();
  textExamples();
 //getCO2() gets the previously read data from the library
   if (rtc.updateTime() == false) //Updates the time variables from RTC
  {
    SerialUSB.print("RTC failed to update");
  }
 String currentDate = rtc.stringDateUSA(); //Get the current date in mm/dd/yyyy format (we're weird)
 //String currentDate = rtc.stringDate(); //Get the current date in dd/mm/yyyy format
  String currentTime = rtc.stringTime(); //Get the time
  myLog.print(currentDate);
  myLog.print(" ");
  myLog.println(currentTime);
  myLog.println("CCS811");
  myLog.print(" CO2 : ");
  myLog.print(myCCS811.getCO2());
  myLog.println(" ppm");

  //getTVOC() gets the previously read data from the library
  myLog.print(" TVOC : ");
  myLog.print(myCCS811.getTVOC());
  myLog.println(" ppb");

  myLog.println("BME280");
  myLog.print(" Temp ");
  myLog.print(myBME280.readTempC(), 2);
  myLog.println(" degrees C");

  myLog.print(" Temp ");
  myLog.print(myBME280.readTempF(), 2);
  myLog.println(" degrees F");

  myLog.print(" Pres ");
  myLog.print(myBME280.readFloatPressure(), 2);
  myLog.println(" Pa");

  myLog.print(" Pres ");
  myLog.print((myBME280.readFloatPressure() * 0.0002953), 2);
  myLog.println(" InHg");

  myLog.print(" Alt ");
  myLog.print(myBME280.readFloatAltitudeMeters(), 2);
  myLog.println("m");

  myLog.print(" Alt ");
  myLog.print(myBME280.readFloatAltitudeFeet(), 2);
  myLog.println("ft");

  myLog.print(" %RH: ");
  myLog.print(myBME280.readFloatHumidity(), 2);
  myLog.println(" %");
      myLog.println(); // Create a white space for easier viewing
    myLog.print("Celsius: ");
    myLog.println(tempC);
    myLog.print("Fahrenheit: ");
    myLog.println(tempF);


  myLog.println();
  myLog.syncFile();
}

//printSensorError gets, clears, then prints the errors
//saved within the error register.
void printSensorError()
{
  uint8_t error = myCCS811.getErrorRegister();

  if (error == 0xFF) //comm error
  {
    SerialUSB.println("Failed to get ERROR_ID register.");
  }
  else
  {
    SerialUSB.print("Error: ");
    if (error & 1 << 5)
      SerialUSB.print("HeaterSupply");
    if (error & 1 << 4)
      SerialUSB.print("HeaterFault");
    if (error & 1 << 3)
      SerialUSB.print("MaxResistance");
    if (error & 1 << 2)
      SerialUSB.print("MeasModeInvalid");
    if (error & 1 << 1)
      SerialUSB.print("ReadRegInvalid");
    if (error & 1 << 0)
      SerialUSB.print("MsgInvalid");
    SerialUSB.println();
  }
}

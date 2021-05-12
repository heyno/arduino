#ifndef __SparkFun_MicroMod_Button_H__
#define __SparkFun_MicroMod_Button_H__

#include <Wire.h>
#include <Arduino.h>

#define DEFAULT_ADDRESS 0x71 //default I2C address of the button
#define DEV_ID 0xAE          //device ID of the Qwiic Button

// Register codes for the Joystick
#define BUTTON_ID                0x00
#define BUTTON_VERSION1          0x01
#define BUTTON_VERSION2          0x02
#define BUTTON_PRESSED           0x03
#define BUTTON_CLICKED           0x04
#define BUTTON_INTERRUPT         0x05
#define BUTTON_DEBOUNCE          0x06
#define BUTTON_CHANGE_ADDREESS   0x1F

// Define the positions of the Interrupt Enable Bits
#define CLICKED_INTERRUPT_ENABLE         0
#define PRESSED_INTERRUPT_ENABLE         1

class MicroModButton
{
public:
    //Device status
    bool begin(uint8_t address = DEFAULT_ADDRESS, TwoWire &wirePort = Wire); //Sets device I2C address to a user-specified address, over whatever port the user specifies.
    bool isConnected();                                                      //Returns true if the button/switch will acknowledge over I2C, and false otherwise
    uint8_t deviceID();                                                      //Return the 8-bit device ID of the attached device.
    bool checkDeviceID();                                                    //Returns true if the device ID matches that of either the button or the switch
    uint16_t getFirmwareVersion();                                           //Returns the firmware version of the attached device as a 16-bit integer. The leftmost (high) byte is the major revision number, and the rightmost (low) byte is the minor revision number.
    bool setI2Caddress(uint8_t address);                                     //Configures the attached device to attach to the I2C bus using the specified address
    uint8_t getI2Caddress();                                                 //Returns the I2C address of the device.
	uint8_t getPressed();
	uint8_t getClicked();
	uint16_t getDebounceTime();             //Returns the time that the button waits for the mechanical contacts to settle, (in milliseconds).
    uint8_t setDebounceTime(uint16_t time); //Sets the time that the button waits for the mechanical contacts to settle (in milliseconds) and checks if the register was set properly. Returns 0 on success, 1 on register I2C write fail, and 2 if the value didn't get written into the register properly.
	bool getPressedInterrupt();
	bool getClickedInterrupt();
	bool setPressedInterrupt(bool bit_setting);
	bool setClickedInterrupt(bool bit_setting);
	uint8_t readSingleRegister(uint8_t reg);    
	uint16_t readDoubleRegister(uint8_t reg);
	bool writeSingleRegister(uint8_t reg, uint8_t data);
	bool writeDoubleRegister(uint8_t reg, uint16_t data);                 //Attempts to write data into a double (two 8-bit) registers. Does not check to make sure it was written successfully. Returns 0 if there wasn't an error on I2C transmission, and 1 otherwise.
private:
    TwoWire *_i2cPort;      //Generic connection to user's chosen I2C port
    uint8_t _deviceAddress; //I2C address of the button/switch
};
#endif
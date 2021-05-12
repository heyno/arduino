#include <Wire.h>
#include <SparkFun_MicroMod_Button.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/*-------------------------------- Device Status ------------------------*/

bool MicroModButton::begin(uint8_t address, TwoWire &wirePort)
{
    _deviceAddress = address; //grab the address that the sensor is on
    _i2cPort = &wirePort;     //grab the port that the user wants to use

    //return true if the device is connected
    return (isConnected());
}

bool MicroModButton::isConnected()
{
    _i2cPort->beginTransmission(_deviceAddress);
    if (_i2cPort->endTransmission() == 0)
        return true;
    return false;
}

uint8_t MicroModButton::deviceID()
{
    return readSingleRegister(BUTTON_ID); //read and return the value in the ID register
}

bool MicroModButton::checkDeviceID()
{
    return (deviceID() == DEV_ID); //Return true if the device ID matches
}

uint16_t MicroModButton::getFirmwareVersion()
{
    uint16_t version = (readSingleRegister(BUTTON_VERSION1)) << 8;
    version |= readSingleRegister(BUTTON_VERSION2);
    return version;
}

bool MicroModButton::setI2Caddress(uint8_t address)
{
    if (address < 0x08 || address > 0x77)
    {
        Serial.println("Error1");
        return 1; //error immediately if the address is out of legal range
    }

    bool success = writeSingleRegister(BUTTON_CHANGE_ADDREESS, address);

    if (success == true)
    {
        _deviceAddress = address;
        return true;
    }

    else
    {
        Serial.println("Error2");
        return false;
    }
}

uint8_t MicroModButton::getI2Caddress()
{
    return _deviceAddress;
}

/*------------------------------ Button Status ---------------------- */

uint8_t MicroModButton::getPressed()
{
	return readSingleRegister(BUTTON_PRESSED);
}

uint8_t MicroModButton::getClicked()
{
    return readSingleRegister(BUTTON_CLICKED);
}

uint16_t MicroModButton::getDebounceTime()
{
    return readDoubleRegister(BUTTON_DEBOUNCE);
}

uint8_t MicroModButton::setDebounceTime(uint16_t time)
{
    return writeDoubleRegister(BUTTON_DEBOUNCE, time);
}

bool MicroModButton::getPressedInterrupt()
{
	uint8_t interrupt = readSingleRegister(BUTTON_INTERRUPT);
	return ((interrupt & (1 << PRESSED_INTERRUPT_ENABLE)) >> PRESSED_INTERRUPT_ENABLE);
}

bool MicroModButton::getClickedInterrupt()
{
	uint8_t interrupt = readSingleRegister(BUTTON_INTERRUPT);
	return ((interrupt & (1 << CLICKED_INTERRUPT_ENABLE)) >> CLICKED_INTERRUPT_ENABLE);
}

bool MicroModButton::setPressedInterrupt(bool bit_setting)
{
	uint8_t interrupt = readSingleRegister(BUTTON_INTERRUPT);
	interrupt &= ~(1 << PRESSED_INTERRUPT_ENABLE); //Clear enable bit
	interrupt |= (bit_setting << PRESSED_INTERRUPT_ENABLE);
	
	return writeSingleRegister(BUTTON_INTERRUPT, interrupt);
}

bool MicroModButton::setClickedInterrupt(bool bit_setting)
{
	uint8_t interrupt = readSingleRegister(BUTTON_INTERRUPT);
	interrupt &= ~(1 << CLICKED_INTERRUPT_ENABLE); //Clear enable bit
	interrupt |= (bit_setting << CLICKED_INTERRUPT_ENABLE);
	
	return writeSingleRegister(BUTTON_INTERRUPT, interrupt);
}

/*------------------------- Internal I2C Abstraction ---------------- */

uint8_t MicroModButton::readSingleRegister(uint8_t reg)
{
    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(reg);
    _i2cPort->endTransmission();

    //typecasting the 1 parameter in requestFrom so that the compiler
    //doesn't give us a warning about multiple candidates
    if (_i2cPort->requestFrom(_deviceAddress, static_cast<uint8_t>(1)) != 0)
    {
        return _i2cPort->read();
    }
    return 0;
}

uint16_t MicroModButton::readDoubleRegister(uint8_t reg)
{ //little endian
    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(reg);
    _i2cPort->endTransmission();

    //typecasting the 2 parameter in requestFrom so that the compiler
    //doesn't give us a warning about multiple candidates
    if (_i2cPort->requestFrom(_deviceAddress, static_cast<uint8_t>(2)) != 0)
    {
        uint16_t data = _i2cPort->read();
        data |= (_i2cPort->read() << 8);
        return data;
    }
    return 0;
}

bool MicroModButton::writeSingleRegister(uint8_t reg, uint8_t data)
{
    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(reg);
    _i2cPort->write(data);
    if (_i2cPort->endTransmission() == 0)
        return true;
    return false;
}

bool MicroModButton::writeDoubleRegister(uint8_t reg, uint16_t data)
{
    _i2cPort->beginTransmission(_deviceAddress);
    _i2cPort->write(reg);
    _i2cPort->write(lowByte(data));
    _i2cPort->write(highByte(data));
    if (_i2cPort->endTransmission() == 0)
        return true;
    return false;
}
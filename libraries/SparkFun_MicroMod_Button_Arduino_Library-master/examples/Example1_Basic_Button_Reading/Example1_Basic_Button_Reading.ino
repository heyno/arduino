#include<SparkFun_MicroMod_Button.h>
#include<Wire.h>

MicroModButton button;

void setup() {
  Serial.begin(115200); 
  Wire.begin();
  delay(100); //Wait for serial port to open
  
  if(!button.begin()) //Connect to the buttons 
  {
    Serial.println("Buttons not found");
    while(1);
  }
  Serial.println("Buttons connected!");
}

void loop() {
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

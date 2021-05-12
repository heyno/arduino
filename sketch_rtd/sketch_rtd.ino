#include <SparkFun_RV8803.h>

void setup() {
  // put your setup code here, to run once:
//The below variables control what the date and time will be set to
int sec = 2;
int minute = 47;
int hour = 14; //Set things in 24 hour mode
int date = 2;
int month = 3;
int year = 2020;
int weekday = 2;
if (rtc.setToCompilerTime() == false) {
Serial.println("Something went wrong setting the time");
}

//Uncomment the below code to set the RTC to your own time
/*if (rtc.setTime(sec, minute, hour, weekday, date, month, year) == false) {
Serial.println("Something went wrong setting the time");
}*/

}

void loop() {
  // put your main code here, to run repeatedly:

}

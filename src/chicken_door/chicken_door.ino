#include <AFMotor.h>
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 RTC;
// constants won't change. They're used here to
// set pin numbers:
const int buttonPin = 2;     // the number of the pushbutton pin
const int closelimit = 14;
const int openlimit = 15;
const int CLOSED = 0;
const int OPEN = 1;
const int UNDETERMINED = 2;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
int doorstate = 0;

AF_DCMotor motor(4, MOTOR12_64KHZ); // create motor #4, 64KHz pwm
 
void setup() {
// initialize the pushbutton pin as an input:
pinMode(buttonPin, INPUT);
pinMode(closelimit, INPUT);
pinMode(openlimit, INPUT);
Serial.begin(57600); // set up Serial library at 9600 bps
motor.setSpeed(255); // set the speed to 200/255

Wire.begin();
RTC.begin();
if (! RTC.isrunning()) {
  Serial.println("RTC is NOT running!");
  // following line sets the RTC to the date & time this sketch was compiled
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}
 
void loop() {
  //DateTime now = RTC.now();
  setdoorstate();
  //checkdoorstatus();
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    Serial.print("Door state is ");
    Serial.println(doorstate);
    if (doorstate == CLOSED) {
      opendoor();
    } else {
      closedoor();
    }
  }
  delay(100);
}  

void setdoorstate() {
  if (digitalRead(closelimit) == LOW) { doorstate = CLOSED; } // checks limit switch and set state accordingly
  else if (digitalRead(openlimit) == LOW) {doorstate = OPEN; }
  else {doorstate = UNDETERMINED; }
}

void closedoor() {
  if(doorstate == CLOSED){
    Serial.println("Door is already Closed, dummy!");
    }else {
    Serial.print("Door is closing ..."); // print out the action of the door:
    motor.run(BACKWARD);
    int counter = 0;
    while ((digitalRead(closelimit) == HIGH) && (counter < 20)) {
      delay(500);// delay to simulate time it take door to close
      counter++;
     // Serial.print(".");
    }
    Serial.println("Door Closed"); 
    delay(1000);
    motor.run(RELEASE);
    Serial.println(" ");
    doorstate = CLOSED;//set door state
    DateTime now = RTC.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  }
}

void opendoor() {
    if(doorstate == OPEN){ 
    Serial.println("Door is already Open!");  
    } else {
     Serial.print("Door is opening ..");// print out the action of the door:
     motor.run(FORWARD);
     int counter = 0;
     while ((digitalRead(openlimit) == HIGH) && (counter < 20)) {
       delay(500);        // delay to simulate time it take door to close
       counter++;
     //Serial.print(".");
     }
     motor.run(RELEASE);
     Serial.println("Door Opened!"); 
     Serial.println(" ");  
     doorstate = OPEN;  //set door state
     DateTime now = RTC.now();
     Serial.print(now.year(), DEC);
     Serial.print('/');
     Serial.print(now.month(), DEC);
     Serial.print('/');
     Serial.print(now.day(), DEC);
     Serial.print(' ');
     Serial.print(now.hour(), DEC);
     Serial.print(':');
     Serial.print(now.minute(), DEC);
     Serial.print(':');
     Serial.print(now.second(), DEC);
     Serial.println();
 }
}

void checkdoorstatus() {
    //setdoorstate();
    if(doorstate == CLOSED){
    Serial.println("Door is Closed");
    Serial.println(" ");
    } else if (doorstate == OPEN){
    Serial.println("Door is Open");
    Serial.println(" ");
    } else {
    Serial.println("Door state undetermined");
    Serial.println(" ");
  }
}

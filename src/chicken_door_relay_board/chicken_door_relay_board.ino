#include <Wire.h>
//#include <DS1307RTC.h>
#include "RTClib.h"
#include <Time.h>
#include <TimeAlarms.h>
#define aref_voltage 3.3         // we tie 3.3V to ARef and measure it with a multimeter!
RTC_DS1307 RTC;
// constants won't change. They're used here to
// set pin numbers:
const int ledRing = 5;
const int buttonPin = 6;     // the number of the pushbutton pin
const int closelimit = 8;
const int openlimit = 7;
const int CLOSED = 0;
const int OPEN = 1;
const int UNDETERMINED = 2;
const int tempTrigger = 25;
int p1 = 9;
int p2 = 10;
int p3 = 11;
int p4 = 12;
int holdTime = 2000;
int previousDir = OPEN;
int tempPin = 0;
int tempReading;
unsigned long time;
long previoustime;
long TimeToCheck = 300;  // Enter milliseconds for unit to check.  1,000 milliseconds = 1 seconds.  E.G. 300,000 millseconds = 300 seconds or 5 minutes
// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
int doorstate = 0;

time_t syncProvider()     //this does the same thing as RTC_DS1307::get()
    {
      return RTC.now().unixtime();
    }
 
void setup() {
  // initialize the pushbutton pin as an input:
  pinMode(ledRing, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(closelimit, INPUT_PULLUP);
  pinMode(openlimit, INPUT_PULLUP);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(9, HIGH);
  digitalWrite(10, HIGH);
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(17, HIGH);
  digitalWrite(16, LOW);
  Serial.begin(57600); // set up Serial library at 9600 bps
  analogReference(EXTERNAL);
  #ifdef AVR
    Wire.begin();
  #else
    Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
  #endif
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
  }
  setSyncProvider(syncProvider); 
  Alarm.alarmRepeat(6, 30, 00, opendoor);
  Alarm.alarmRepeat(22, 00, 00, closedoor);
  Alarm.timerRepeat(15, checkTemp); 
  Alarm.timerOnce(10, opendoor);
}
 
void loop() {
  setdoorstate();
  //checkdoorstatus();
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    Serial.print("Door state is ");
    switch (doorstate) {
      case 0:
        Serial.println("closed");
        break;
      case 1:
        Serial.println("open");
        break;
      default:
        Serial.println("undetermined");
    }
    if (doorstate == OPEN) {
      closedoor();
    } else if (doorstate == CLOSED) {
      opendoor();
    } else {
      if (previousDir == CLOSED) {
        opendoor();
      } else {
        closedoor();
      }
    }
  }
  delay(100);
}

void checkTemp() {
  tempReading = analogRead(tempPin);
  float voltage = tempReading * aref_voltage;
  voltage /= 1024.0;
  float temperatureC = (voltage - 0.5) * 100;
  Serial.print(temperatureC); Serial.println(" degrees C");
  if (temperatureC >= tempTrigger) {
    digitalWrite(p4, LOW);
  } else {
    digitalWrite(p4, HIGH);
  }
}

void setdoorstate() {
  if (digitalRead(closelimit) == LOW) {doorstate = CLOSED; } // checks limit switch and set state accordingly
  else if (digitalRead(openlimit) == LOW) {doorstate = OPEN; }
  else {doorstate = UNDETERMINED; }
}

void closedoor() {
  if(doorstate == CLOSED){
    Serial.println("Door is already Closed!");
  } else {
    Serial.print("Door is closing ..."); // print out the action of the door:
    monitorLimit(p2, closelimit);
    setdoorstate();
    if (doorstate == UNDETERMINED) {
      previousDir = CLOSED;
    } else {
      Serial.println("Door Closed"); 
      Serial.println(" ");
      doorstate = CLOSED;//set door state
    }
    printTime();
  }
}

void opendoor() {
  if(doorstate == OPEN){ 
    Serial.println("Door is already Open!");  
  } else {
    digitalWrite(13, HIGH);
    Serial.println("Door is opening...");
    monitorLimit(p1, openlimit);
    if (doorstate == UNDETERMINED) {
      previousDir = OPEN;
    } else {
      Serial.println("Door Open");
      Serial.println(" ");
      doorstate = OPEN;  //set door state
      digitalWrite(13, LOW);
    }
    printTime();
  }
}

void monitorLimit(int relay, int limit) {
  digitalWrite(relay, LOW);
  digitalWrite(ledRing, HIGH);
  int counter = 0;
  delay(500);
  while ((digitalRead(openlimit) == HIGH) && digitalRead(closelimit) == HIGH && (counter < 10000)) {
    if (digitalRead(buttonPin) == HIGH) {
      delay(50);
      break;
    }
    delay(20);        // delay to simulate time it take door to close
    counter++;
  }
  digitalWrite(relay, HIGH);
  digitalWrite(ledRing, LOW);
  digitalWrite(13, LOW);
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

void printTime() {
  //setSyncProvider(RTC.get);
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

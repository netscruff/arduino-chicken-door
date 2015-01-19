#include <Wire.h>
#include "RTClib.h"
#include <Time.h>
#include <TimeAlarms.h>
#define aref_voltage 3.3      // we tie 3.3V to ARef and measure it with a multimeter!
RTC_DS1307 RTC;
// constants won't change. They're used here to
// set pin numbers:
const int ledRing = 5;        // led ring in the pushbutton
const int buttonPin = 6;      // number of the pushbutton pin
const int closelimit = 8;     // limit switch for the bottom of the frame
const int openlimit = 7;      // limit switch for the top of the frame
const int CLOSED = 0;         // door is open
const int OPEN = 1;           // door is closed
const int UNDETERMINED = 2;   // door neither open or closed
const int tempTrigger = 4;   // temperature trigger to turn on relay for fan
const int tempPin = 0;        // analog temperature sensor
// relay board with 4 relays
const int p1 = 9;             // door motor control
const int p2 = 10;            // door motor control
const int p3 = 11;            // unused
const int p4 = 12;            // fan control

// variables will change:
int tempReading;
int previousDir = OPEN;
int buttonState = 0;          // variable for reading the pushbutton status
int doorstate = 0;

time_t syncProvider() {       //this does the same thing as RTC_DS1307::get()
  return RTC.now().unixtime();
}
 
void setup() {
  pinMode(ledRing, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(closelimit, INPUT_PULLUP);
  pinMode(openlimit, INPUT_PULLUP);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(17, OUTPUT);         // Used to power the RTC
  pinMode(16, OUTPUT);         // Ground for RTC
  // turn all the relays off by setting HIGH
  digitalWrite(9, HIGH);
  digitalWrite(10, HIGH);
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(17, HIGH);
  digitalWrite(16, LOW);
  Serial.begin(57600); // set up Serial library at 57600 bps
  analogReference(EXTERNAL);    // Used for the temperature probe
  #ifdef AVR
    Wire.begin();
  #else
    Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
  #endif
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
  }
  // following line sets the RTC to the date & time this sketch was compiled
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  //RTC.adjust(DateTime(2015, 1, 10, 11, 45, 0));
  setSyncProvider(syncProvider);
  RTC.now;
  Alarm.alarmRepeat(6, 30, 00, opendoor);  //open door in the morning
  Alarm.alarmRepeat(18, 30, 00, closedoor); // close door in the evening
  Alarm.timerRepeat(15, checkTemp); // check temerature every 15 seconds
  Alarm.timerOnce(10, opendoor);  // 10 seconds after poweron, open up the door
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
  Alarm.delay(100);
}

void checkTemp() {
  tempReading = analogRead(tempPin);
  float voltage = tempReading * aref_voltage;
  voltage /= 1024.0;
  float temperatureC = (voltage - 0.5) * 100;
  printTime();
  Serial.print(temperatureC); Serial.println(" degrees C");
  if (temperatureC <= tempTrigger && doorstate == CLOSED) {
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

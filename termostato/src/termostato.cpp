/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/IOT/termostato/src/termostato.ino"
#include <DS18B20.h>
/*
 * Project termostato
 * Description:
 * Author:
 * Date:
 */

// setup() runs once, when the device is first turned on.

void setup();
void loop();
void registerWrite();
void getTemp();
int setPoint(String point);
#line 11 "c:/IOT/termostato/src/termostato.ino"
int button, shiftStatus, relay1;
int latchPin = A3;
int dataPin = A4;
int clockPin = A2;
const int MAXRETRY = 4;
const int16_t dsData = D7;
const uint32_t msSAMPLE_INTERVAL = 5000;
double soglia = 0;
double actualTemp = 0;

DS18B20  ds18b20(dsData, true); 

double celsius;
uint32_t msLastMetric;
uint32_t msLastSample;

void setup() {
  // Put initialization like pinMode and begin functions here.
  pinMode(D3, INPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  Particle.variable("bottone", button);
  Particle.variable("soglia", soglia);
  Particle.variable("Tempreatura attuale", actualTemp);
  Particle.function("setSoglia", setPoint);
  Particle.variable("Relay", relay1);

  Serial.begin(9600);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  button = digitalRead(D3);
  if (millis() - msLastSample >= msSAMPLE_INTERVAL){
    getTemp();
    actualTemp = celsius;
  }

  if(button == 1){
    if(actualTemp < soglia){
      relay1 = 1;
      registerWrite();
    }else{
      relay1 = 0;
      registerWrite();
    }
  }else{
    relay1 = 0;
    registerWrite();
  }

}

void registerWrite() {

    shiftStatus = 8*relay1;

    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, shiftStatus);
    digitalWrite(latchPin, HIGH);
}

void getTemp(){
  float _temp;
  int   i = 0;

  do {
    _temp = ds18b20.getTemperature();
  } while (!ds18b20.crcCheck() && MAXRETRY > i++);

  if (i < MAXRETRY) {
    celsius = _temp;
  }
  else {
    celsius = NAN;
    Serial.println("Invalid reading");
  }
  msLastSample = millis();
}

int setPoint(String point){
  soglia = point.toFloat();
  return 1;
}
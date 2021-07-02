#include <DS18B20.h>
/*
 * Project primo
 * Description:
 * Author:
 * Date:
 */

// setup() runs once, when the device is first turned on.
int button, shiftStatus, relay1, relay2, relay3, relay4, oldButton=0;
int latchPin = A3;
int dataPin = A4;
int clockPin = A2;
const int MAXRETRY = 4;
const int16_t dsData = D7;
const uint32_t msSAMPLE_INTERVAL = 2500;
const uint32_t msTEMP_INTERVAL = 5000;

DS18B20  ds18b20(dsData, true); 

double   celsius;
double   fahrenheit;
uint32_t msLastMetric;
uint32_t msLastSample;
uint32_t msLastTemp;
uint32_t msRelay = 0;
int timeout = 0;
int status = 0;

void setup() {
  // Put initialization like pinMode and begin functions here.
  pinMode(D3, INPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  Particle.variable("bottone", button);
  Particle.function("attivaRelay", attivaRelay);

  Serial.begin(9600);

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  button = digitalRead(D3);

  if(oldButton==button){
    delay(0);
    //Serial.println(timeout);
  }else{
    relay1 = button;
    registerWrite();
    oldButton=button;
    timeout = millis();
    Serial.println(timeout);
    //timeout = 0;
  } 

  if ((timeout > 0)&&(millis() - timeout >= 5000)){
    relay3 = button;
    registerWrite();
    timeout = 0;
  }


  if (millis() - msLastSample >= msSAMPLE_INTERVAL){
    getTemp();
  }

  if (millis() - msLastTemp >= msTEMP_INTERVAL){
      Serial.println(celsius);
      msLastTemp=millis();
  }

  if ((status==0)&&(millis() - msRelay >= 5000)){
    relay4 = 1;
    status = relay4;
    msRelay = millis();
    registerWrite();
  }else if((status == 1)&&(millis() - msRelay >= 5000)){
    relay4 = 0;
    status = relay4;
    msRelay = millis();
    registerWrite();
  }

}

void registerWrite() {

    shiftStatus = 8*relay1 + 16*relay2 + 32*relay3 + 64*relay4;

    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, shiftStatus);
    digitalWrite(latchPin, HIGH);
}

int attivaRelay(String relay){
  relay2 = relay.toInt();
  registerWrite();
  return 1;
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
    celsius = fahrenheit = NAN;
    Serial.println("Invalid reading");
  }
  msLastSample = millis();
}
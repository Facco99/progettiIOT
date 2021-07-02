/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/IOT/progetti/sensoreUmidita/src/sensoreUmidita.ino"
#include <Wire.h>
#include "DFRobot_SHT20.h"
#include <Grove_ChainableLED.h>
#include <math.h>
#include "Adafruit_VEML7700.h"
#include "SHT31.h"

void setup();
void loop();
int modValori(String valore);
#line 8 "c:/IOT/progetti/sensoreUmidita/src/sensoreUmidita.ino"
#define NUM_LEDS  1

ChainableLED leds(D2, D3, NUM_LEDS);

Adafruit_VEML7700 veml;

SHT31 sht31 = SHT31();

std::string s;

/*
 * Project sensoreUmidita
 * Description:
 * Author:
 * Date:
 */

const uint32_t msUm_CONST = 5000;
uint32_t msVal, msLed, msTimer,pubTimer = 60000, n, msBagna;
int status;
double humdTerra, tempTerra, humdTerraLunga, tempTerraLunga, lum, lumLunga, humdAmb, tempAmb, humdAmbLunga, tempAmbLunga;
String json, bagna;

DFRobot_SHT20 sht20;

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  Serial.begin(9600);
  leds.init();
  sht20.initSHT20();
  delay(100);
  sht20.checkSHT20();
  Particle.variable("Umidità Terra", humdTerra);
  Particle.variable("Temperatura Terra", tempTerra);
  Particle.variable("Umidità Amb", humdAmb);
  Particle.variable("Temperatura Amb", tempAmb);
  //Particle.variable("Soglia", soglia);
  Particle.variable("Luminosità", lum);
  //Particle.function("modUmidità", modUm);
  Particle.function("Valori Publish", modValori);

  sht31.begin();

  if (!veml.begin()){
    Serial.println("Sensore non trovato");
    while (1);
  }

  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_800MS);
  veml.setLowThreshold(10000);
  veml.setHighThreshold(20000);
  veml.interruptEnable(true);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  if(millis()-msVal>=msUm_CONST){
    humdTerraLunga = sht20.readHumidity();                 
    tempTerraLunga = sht20.readTemperature();
    humdAmbLunga = sht31.getHumidity(); 
    tempAmbLunga = sht31.getTemperature();

    humdTerra = ((int) (humdTerraLunga*100))/100.0;
    tempTerra = ((int) (tempTerraLunga*100))/100.0;
    humdAmb = ((int) (humdAmbLunga*100))/100.0;
    tempAmb = ((int) (tempAmbLunga*100))/100.0;

    lumLunga = veml.readLux();
    lum = ((int) (lumLunga*100))/100.0;

    json = "{\"temperatura_terra\": "+String(tempTerra)+",\"umidita_terra\": "+String(humdTerra)+",\"luminosita\": "+String(lum)+",\"temperatura_ambiente\": "+String(tempAmb)+",\"umidita_ambiente\": "+String(humdAmb)+"}";

    msVal = millis();
  }

  /* if(humdTerra<soglia){
    bagna = "Bagnare la pianta!!";
    if ((status==0)&&(millis() - msLed >= 1000)){
      leds.setColorRGB(0, 255, 0, 255);
      status = 1;
      msLed = millis();
    }else if((status == 1)&&(millis() - msLed >= 1000)){
      leds.setColorRGB(0, 0, 0, 0);
      status = 0;
      msLed = millis();
    }
  }else{
    bagna="La pianta non ha bisogno di acqua";
    leds.setColorRGB(0, 0, 0, 0);
  } */

  if(millis()-msTimer >= pubTimer){
    Particle.publish("Publish", json, PRIVATE);
    msTimer = millis();
  }

}

/* int modUm(String valore){
  soglia = valore.toFloat();
  return 1;
} */

int modValori(String valore){
  s = valore;
  if(s.size()>0){
    std::string delimiter = "=";
    std::string type = s.substr(0, s.find(delimiter));
    std::string number = s.substr(s.find(delimiter)+1);

    n = atoi( number.c_str() );

    if(type == "publish"){
      pubTimer = n;
    }else{
      return -1;
    }
  }
  return 1;
}
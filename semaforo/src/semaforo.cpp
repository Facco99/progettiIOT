/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/IOT/progetti/semaforo/src/semaforo.ino"
#include <Grove_ChainableLED.h>

void setup();
void loop();
#line 3 "c:/IOT/progetti/semaforo/src/semaforo.ino"
#define NUM_LEDS  2

ChainableLED leds(D2, D3, NUM_LEDS);

/*
 * Project semaforo
 * Description:
 * Author:
 * Date:
 */
const uint32_t verde1 = 5000;
const uint32_t arancione1 = 2500;
const uint32_t verde2 = 5000;
const uint32_t arancione2 = 2500;

/* String arr1[]={"rosso","giallo","verde"};
String arr2[]={"verde","giallo","rosso"}; */


uint32_t msSemaforo;
uint32_t msButton;

int status = 0, button, pulsante, variabile;

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  Serial.begin(9600);
  leds.init();
  pinMode(D4, INPUT);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  button = digitalRead(D4);
  if(button == 1){
    pulsante = 1;
  }
  Serial.print("Pulsante = ");
  Serial.println(pulsante);

  if((status==0)&&(millis()-msSemaforo >= verde1)){
    leds.setColorRGB(0,0,255,0);
    leds.setColorRGB(1,255,0,0);
    msSemaforo = millis();
    status = 1;
   }
  if((status==1)&&(millis()-msSemaforo >= arancione1)){
    leds.setColorRGB(0,252, 136, 3);
    leds.setColorRGB(1,255,0,0);
    msSemaforo = millis();
    if(pulsante == 1){
      status = 4;
    }else{
      status = 2;
      variabile = 0;
    }
  }
  if((status==2)&&(millis()-msSemaforo >= verde2)){
    leds.setColorRGB(0,255,0,0);
    leds.setColorRGB(1,0,255,0);
    msSemaforo = millis();
    status=3;
  }
  if((status==3)&&(millis()-msSemaforo >= arancione2)){
    leds.setColorRGB(0,255,0,0);
    leds.setColorRGB(1,252, 136, 3);
    msSemaforo = millis();
    if(pulsante == 1){
      status = 4;
    }else{
      variabile = 2;
      status = 0;
    }
  }
  if((status==4)&&(millis()-msSemaforo >= 5000)){
    leds.setColorRGB(0,255,0,0);
    leds.setColorRGB(1,255, 0, 0);
    msSemaforo = millis();
    pulsante = 0;
    status = variabile;
  }
  
}
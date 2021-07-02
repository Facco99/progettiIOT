/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/IOT/progetti/ledRGB/src/ledRGB.ino"
#include <Grove_ChainableLED.h>
 
void setup();
void loop();
#line 3 "c:/IOT/progetti/ledRGB/src/ledRGB.ino"
#define NUM_LEDS  1

ChainableLED leds(D2, D3, NUM_LEDS);

/*
 * Project ledRGB
 * Description:
 * Author:
 * Date:
 */

  const uint32_t msPotTimeout = 15000;
  int potRosso, potBlu, potVerde;
  uint32_t msPot;

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  Serial.begin(9600);
  leds.init();

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.

  potBlu = analogRead(A0);
  potRosso = analogRead(A1);
  potVerde = analogRead(A5);


  if(millis() - msPot >= msPotTimeout){
    Serial.print("Potenziometro blu = ");
    Serial.println(potBlu);
    Serial.print("Potenziometro rosso = ");
    Serial.println(potRosso);
    Serial.print("Potenziometro verde = ");
    Serial.println(potVerde);
    msPot = millis();
  }

  int colorR = potRosso/16;
  int colorG = potVerde/16;
  int colorB = potBlu/16;

  leds.setColorRGB(0, colorR, colorG, colorB);
}
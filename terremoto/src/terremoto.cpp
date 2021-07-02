/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/IOT/progetti/terremoto/src/terremoto.ino"
#include <Grove_ChainableLED.h>
 
void setup();
void loop();
#line 3 "c:/IOT/progetti/terremoto/src/terremoto.ino"
#define NUM_LEDS  1

ChainableLED leds(D2, D3, NUM_LEDS);

/*
 * Project terremoto
 * Description:
 * Author:
 * Date:
 */

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  Serial.begin(9600);
  leds.init();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  int sensorState = digitalRead(D5);
  Serial.println(sensorState);
  delay(100);
    if(sensorState == HIGH){
        leds.setColorRGB(0, 255, 0, 0);
    }
    else{
        leds.setColorRGB(0, 0, 255, 0);
    }


}
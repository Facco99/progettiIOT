#include <Grove_ChainableLED.h>
 
#define NUM_LEDS  1

ChainableLED leds(D2, D3, NUM_LEDS);

/*
 * Project ledRGB
 * Description:
 * Author:
 * Date:
 * LED RGB = D2, D3
 * POT BLU = A0
 * POT ROSSO = A1
 * POT VERDE = A5
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
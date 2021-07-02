/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/IOT/progetti/ledRGB_seriale/src/ledRGB_seriale.ino"
#include <Grove_ChainableLED.h>
 
void setup();
void loop();
#line 3 "c:/IOT/progetti/ledRGB_seriale/src/ledRGB_seriale.ino"
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

String comando, vocale;
std::string s;
int n;
int r, g ,b;
String intermediate;

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  Serial.begin(9600);
  leds.init();
  Serial.setTimeout(100);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.

  /* vocale = Serial.readStringUntil('='); */
  /* comando = Serial.readString(); */
  s = Serial.readString();
  if(s.size()>0){
    std::string delimiter = "=";
    std::string letter = s.substr(0, s.find(delimiter));
    std::string number = s.substr(s.find(delimiter)+1);

    Serial.println(letter.c_str());
    Serial.println(number.c_str());

    n = atoi( number.c_str() );

    if(letter == "r"){
      r=n;
      leds.setColorRGB(0, r, g, b);
    }else if(letter == "g"){
      g=n;
      leds.setColorRGB(0, r, g, b);
    }else if(letter == "b"){
      b=n;
      leds.setColorRGB(0, r, g, b);
    }else if(letter == "o"){
      r=0;
      g=0;
      b=0;
      leds.setColorRGB(0, r, g, b);
    }else{
      Serial.println("Errore, comandi disponibili: ");
      Serial.println("r ROSSO, b BLU, g VERDE, o SPEGNI");
    }

  }

  /* Serial.println(vocale); */
  /* Serial.println(comando); */

  /* if(vocale == "r"){
    r=comando.toInt();
    leds.setColorRGB(0, r, g, b);
  }else if(vocale == "g"){
    g=comando.toInt();
    leds.setColorRGB(0, r, g, b);
  }else if(vocale == "b"){
    b=comando.toInt();
    leds.setColorRGB(0, r, g, b);
  }else if(vocale == "o"){
    r=0;
    g=0;
    b=0;
    leds.setColorRGB(0, r, g, b);
  } */
  
}
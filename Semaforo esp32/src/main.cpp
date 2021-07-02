#include <Arduino.h>
#include "KMPProDinoESP32.h"
#include "KMPCommon.h"

const uint32_t verde = 10000;
const uint32_t arancio = 5000;
const uint32_t rosso = 15000;
const uint32_t lampeggio = 1000;

uint32_t msSemaforo;
uint32_t msLampeggio;

int status = 0, pedone, lamp= 0;

void setup() {
  // put your setup code here, to run once:
  delay(5000);
	Serial.begin(115200);
	Serial.println("Semaforo example is starting...");
	KMPProDinoESP32.setStatusLed(blue);

	// Init Dino board. Set pins, start W5500.
	// Init Dino board.
	KMPProDinoESP32.begin(ProDino_ESP32);
	//KMPProDinoESP32.begin(ProDino_ESP32_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa_RFM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);

	KMPProDinoESP32.offStatusLed();
  Serial.println("Semaforo example started...");
}

void loop() {
  // put your main code here, to run repeatedly:

  if(KMPProDinoESP32.getOptoInState(0)==1){
    pedone = 1;
  }

  if(KMPProDinoESP32.getOptoInState(1)==0){
    if((status==0)&&(millis()-msSemaforo>=verde)){
    KMPProDinoESP32.setRelayState(1, true);
    KMPProDinoESP32.setRelayState(2, false);
    KMPProDinoESP32.setRelayState(3, false);
    if(pedone == 1){
      status = 2;
      pedone = 0;
    }else{
      status = 1;
    }
    msSemaforo = millis();
    }else if ((status==1)&&(millis()-msSemaforo>=arancio)){
      KMPProDinoESP32.setRelayState(2, true);
      KMPProDinoESP32.setRelayState(1, false);
      KMPProDinoESP32.setRelayState(3, false);
      status = 2;
      msSemaforo = millis();
    }else if ((status==2)&&(millis()-msSemaforo>=rosso)){
      KMPProDinoESP32.setRelayState(3, true);
      KMPProDinoESP32.setRelayState(2, false);
      KMPProDinoESP32.setRelayState(1, false);
      status = 0;
      msSemaforo = millis();
    }
  }else{
    KMPProDinoESP32.setRelayState(1, false);
    KMPProDinoESP32.setRelayState(3, false);

    if((lamp == 0)&&(millis()-msLampeggio>=lampeggio)){
      KMPProDinoESP32.setRelayState(2, true);
      lamp = 1;
      msLampeggio = millis();
    }else if((lamp == 1)&&(millis()-msLampeggio>=lampeggio)){
      KMPProDinoESP32.setRelayState(2, false);
      lamp = 0;
      msLampeggio = millis();
    }
  }

  /* Decommentare per spegnere il programma
  KMPProDinoESP32.setRelayState(3, false);
  KMPProDinoESP32.setRelayState(2, false);
  KMPProDinoESP32.setRelayState(1, false); 
  */

}
#include <Arduino.h>
#include "KMPProDinoESP32.h"
#include "KMPCommon.h"

const uint32_t relayAcceso = 5000;

uint32_t acceso;

void setup()
{
	delay(5000);
	Serial.begin(115200);
	Serial.println("Inputs example is starting...");
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
  Serial.println("Inputs example started...");
}

void loop() {
  // put your main code here, to run repeatedly:

  if((KMPProDinoESP32.getOptoInState(0) == 1)){
    KMPProDinoESP32.setRelayState(1, true);
  }else{
    KMPProDinoESP32.setRelayState(1, false);
  }

  if(KMPProDinoESP32.getOptoInState(1) == 1){
    for (size_t i = 2; i < RELAY_COUNT; i++)
    {
      KMPProDinoESP32.setRelayState(i, true);
      delay(1000);
    }

    for (size_t i = 2; i < RELAY_COUNT; i++)
    {
      KMPProDinoESP32.setRelayState(i, false);
      delay(1000);
    }
  }else{
    KMPProDinoESP32.setRelayState(2, false);
    KMPProDinoESP32.setRelayState(3, false);
  }
}
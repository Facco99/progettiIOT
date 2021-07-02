#include <Arduino.h>
#include <ModbusMaster.h>
#include "KMPProDinoESP32.h"
#include "KMPCommon.h"

#define MAX485_DE_RE 2

uint32_t msRead;
int premuto;

// instantiate ModbusMaster object
ModbusMaster master;


void preTransmission(){
  digitalWrite(MAX485_DE_RE, HIGH);
  delayMicroseconds(70);
}

void postTransmission(){
  RS485Serial.flush();
  delayMicroseconds(1562);
  digitalWrite(MAX485_DE_RE, LOW);
}

void setup()
{
  pinMode(MAX485_DE_RE, OUTPUT);
  digitalWrite(MAX485_DE_RE, LOW);

  KMPProDinoESP32.begin(ProDino_ESP32);

  Serial.begin(115200);
  KMPProDinoESP32.rs485Begin(9600);

  Serial.println("Modbus starting...");

  // communicate with Modbus slave ID 11 over Serial (port 1)
  master.begin(11, RS485Serial);
  master.preTransmission(preTransmission);
  master.postTransmission(postTransmission);
  delay(1000);
  Serial.println("Modbus start...");
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t result, j;

  if((KMPProDinoESP32.getOptoInState(0)==1)&&(premuto == 0)){
    premuto = 1;
  }else if ((premuto ==2)&&(KMPProDinoESP32.getOptoInState(0)==0)){
    premuto = 3;
  }

  if(premuto==1){
    Serial.println("Bottone premuto");
    Serial.println("Tutti i relay sono stati accesi");
    master.setTransmitBuffer(0, 1);
    master.setTransmitBuffer(1, 1);
    master.setTransmitBuffer(2, 1);
    master.setTransmitBuffer(3, 1);
    master.setTransmitBuffer(4, 1);
    master.setTransmitBuffer(5, 1);
    delay(100);
    result = master.writeMultipleRegisters(38, 6);
    premuto = 2;
  }else if(premuto==3){
    Serial.println("Bottone lasciato");
    Serial.println("Tutti i relay sono stati spenti");
    master.setTransmitBuffer(0, 0);
    master.setTransmitBuffer(1, 0);
    master.setTransmitBuffer(2, 0);
    master.setTransmitBuffer(3, 0);
    master.setTransmitBuffer(4, 0);
    master.setTransmitBuffer(5, 0);
    delay(100);
    result = master.writeMultipleRegisters(38, 6);
    premuto = 0;
  }

  delay(100);
  result = master.readHoldingRegisters(38,6);

  if ((result == master.ku8MBSuccess)&&(millis()-msRead>=10000))
  {
    for (j = 0; j < 6; j++)
    {
      //data[j] = master.getResponseBuffer(j);
      Serial.println(master.getResponseBuffer(j));
    }
    msRead = millis();
  }
}
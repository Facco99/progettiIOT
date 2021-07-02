#include <Arduino.h>
#include <ModbusMaster.h>
#include "KMPProDinoESP32.h"
#include "KMPCommon.h"

#define MAX485_DE_RE 2

const uint32_t post = 1562;

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

  //RS485Serial.begin(9600);

  Serial.println("Modbus starting...");

  // communicate with Modbus slave ID 11 over Serial (port 1)
  master.begin(11, RS485Serial);
  master.preTransmission(preTransmission);
  master.postTransmission(postTransmission);
  delay(1000);
  Serial.println("Modbus start...");
}

void loop()
{
  uint8_t j, result;
  uint16_t data[6];

  for (size_t i = 39; i <= 44; i++)
  {
    result = master.writeSingleRegister(i, 1);
    delay(1000);
  }

  result = master.readHoldingRegisters(39, 2);
  Serial.print("Read: ");
  Serial.println(result);

  if (result == master.ku8MBSuccess)
  {
    for (j = 0; j < 6; j++)
    {
      //data[j] = master.getResponseBuffer(j);
      Serial.println(master.getResponseBuffer(j));
    }
  }

  // Switch every relay to Off
  for (size_t i = 39; i <= 44; i++)
  {
    result = master.writeSingleRegister(i, 0);
    delay(1000);
  }
  Serial.print("Write: ");
  Serial.println(result);

  result = master.readHoldingRegisters(39, 2);
  Serial.print("Read: ");
  Serial.println(result);

  if (result == master.ku8MBSuccess)
  {
    for (j = 0; j < 6; j++)
    {
      //data[j] = master.getResponseBuffer(j);
      Serial.println(master.getResponseBuffer(j));
    }
  }

  /* preTransmission();

  RS485Serial.write("Prova");

  postTransmission();

  delay(1000); */
}
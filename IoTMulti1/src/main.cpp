#include <Arduino.h>
#include <ModbusMaster.h>

#define MAX485_DE_RE 14
#define CONFIG_MB_UART_TXD 4
#define CONFIG_MB_UART_RXD 36

#define RED 32
#define GREEN 33
#define BLU 12

float temp;
int soglia = 23;

// instantiate ModbusMaster object
ModbusMaster master;

void preTransmission(){
  digitalWrite(MAX485_DE_RE, HIGH);
  delayMicroseconds(70);
}

void postTransmission(){
  Serial1.flush();
  delayMicroseconds(1562);
  digitalWrite(MAX485_DE_RE, LOW);
}

void setup()
{
  pinMode(MAX485_DE_RE, OUTPUT);
  digitalWrite(MAX485_DE_RE, LOW);

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLU, OUTPUT);

  Serial.begin(115200);

  Serial1.begin(115200, SERIAL_8N1, CONFIG_MB_UART_RXD, CONFIG_MB_UART_TXD);

  Serial.println("IoTMulti starting...");

  // communicate with Modbus slave ID 11 over Serial (port 1)
  master.begin(10, Serial1);
  master.preTransmission(preTransmission);
  master.postTransmission(postTransmission);
  delay(1000);
  Serial.println("IoTMulti start...");
}

void loop() {
  // put your main code here, to run repeatedly:

  uint8_t result, j;
  for(int i = 0; i<4; i++){
    master.setTransmitBuffer(i,1);
  }
  delay(500);
  result = master.writeMultipleRegisters(304,4);
  delay(500);
  result = master.readHoldingRegisters(304,4);
  Serial.print("Acceso = ");
  Serial.println(result);
  if (result == master.ku8MBSuccess)
  {
    for (j = 0; j < 4; j++)
    {
      //data[j] = master.getResponseBuffer(j);
      Serial.println(master.getResponseBuffer(j));
    }
  }

  for(int i = 0; i<4; i++){
    master.setTransmitBuffer(i,0);
  }
  delay(500);
  result = master.writeMultipleRegisters(304,4);
  delay(500);
  result = master.readHoldingRegisters(304,4);
  Serial.print("Spento = ");
  Serial.println(result);
  if (result == master.ku8MBSuccess)
  {
    for (j = 0; j < 4; j++)
    {
      //data[j] = master.getResponseBuffer(j);
      Serial.println(master.getResponseBuffer(j));
    }
  }

  delay(100);
  result = master.readHoldingRegisters(326,1);

  if(result == master.ku8MBSuccess){
    temp = master.getResponseBuffer(0)/100.00;
    Serial.println(temp);
  }
}
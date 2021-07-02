#include <Arduino.h>
#include "arduino_secrets.h"
#include <ModbusMaster.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

//Pin per la comunicazione RS485
#define MAX485_DE_RE 14
#define CONFIG_MB_UART_TXD 4
#define CONFIG_MB_UART_RXD 36

//Variabili per il modbus
uint16_t data[26];        //array dove salvo tutti i registri della scheda
uint8_t digitalOutput[4]; //digitalOutput (Relays)
int digitalInput[4];      //digitalInput (Bottoni)
uint8_t analogOutput[4];  //analogOutput
float analogInput[4];     //analogInput
float sonda[4];           //sonde di temperatura

// Inizalizzo la libreia ModBus
ModbusMaster master;

//Funzioni per il MODBUS
void preTransmission()
{ //Questa viene eseguida prima di aprire la comunicazione rs485
  digitalWrite(MAX485_DE_RE, HIGH);
  delayMicroseconds(70);
}

void postTransmission()
{ //Questa viene eseguita dopo la chiusura della comunicazione rs485
  Serial1.flush();
  delayMicroseconds(1562);
  digitalWrite(MAX485_DE_RE, LOW);
}

void readModbus()
{
  uint8_t result, i;

  delay(100);
  result = master.readHoldingRegisters(304, 26);

  if (result == master.ku8MBSuccess)
  {
    for (i = 0; i < 26; i++)
    {
      data[i] = master.getResponseBuffer(i);
    }
  }
}
/**
 * @brief Funzione per scrivere sui registri digitali
 * @param reg Numero di registro di partenza
 * @param qta Quanti registri vuoi che la funzione scriva
 * @param val Valore da scrivere (1 = ON, 0 = OFF)
 */
void writeDigital(int reg, int qta, int val)
{

  if ((val < 0) || (val > 1))
  {
    return;
  }

  for (int j = 0; j < qta; j++)
  {
    master.setTransmitBuffer(j, val);
  }

  delay(100);
  master.writeMultipleRegisters(reg, qta);
}

const char *PARAM_INPUT_1 = "output";
const char *PARAM_INPUT_2 = "state";
String nomiCampi[20];

byte _mac[] = {0x00, 0x08, 0xDC, 0x11, 0x27, 0x8C};
// Ip address dipende dalla network su cui stai lavorando.
IPAddress _ip(192, 168, 1, 197);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with button section in your web page
String processor(const String &var)
{
  //Serial.println(var);
  if (var == "BUTTONPLACEHOLDER")
  {
    String buttons = "";
    buttons += "<h4>Relay1</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"1\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Relay2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Relay3</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"3\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Relay4</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\"><span class=\"slider\"></span></label>";
    return buttons;
  }
  else if (var == "INPUTPLACEHOLDER")
  {
    String inputs = "";
    inputs += "<form method=\"post\">";
    inputs += "<label>Relay1:</label><input type=\"text\" name=\"relay1\">";
    inputs += "<br><label>Relay2:</label><input type=\"text\" name=\"relay2\">";
    inputs += "<br><label>Relay4:</label><input type=\"text\" name=\"relay4\">";
    inputs += "<br><label>Relay3:</label><input type=\"text\" name=\"relay3\">";
    inputs += "<br><label>AnalogOutput1:</label><input type=\"text\" name=\"anOut1\">";
    inputs += "<br><label>AnalogOutput2:</label><input type=\"text\" name=\"anOut2\">";
    inputs += "<br><label>AnalogOutput3:</label><input type=\"text\" name=\"anOut3\">";
    inputs += "<br><label>AnalogOutput4:</label><input type=\"text\" name=\"anOut4\">";
    inputs += "<br><label>DigitalInput1:</label><input type=\"text\" name=\"dIn1\">";
    inputs += "<br><label>DigitalInput2:</label><input type=\"text\" name=\"dIn2\">";
    inputs += "<br><label>DigitalInput3:</label><input type=\"text\" name=\"dIn3\">";
    inputs += "<br><label>DigitalInput4:</label><input type=\"text\" name=\"dIn4\">";
    inputs += "<br><label>AnalogInput1:</label><input type=\"text\" name=\"anIn1\">";
    inputs += "<br><label>AnalogInput2:</label><input type=\"text\" name=\"anIn2\">";
    inputs += "<br><label>AnalogInput3:</label><input type=\"text\" name=\"anIn3\">";
    inputs += "<br><label>AnalogInput4:</label><input type=\"text\" name=\"anIn4\">";
    inputs += "<br><label>Sonda1:</label><input type=\"text\" name=\"sonda1\">";
    inputs += "<br><label>Sonda2:</label><input type=\"text\" name=\"sonda2\">";
    inputs += "<br><label>Sonda3:</label><input type=\"text\" name=\"sonda3\">";
    inputs += "<br><label>Sonda4:</label><input type=\"text\" name=\"sonda4\">";
    inputs += "<br><input type=\"submit\" value=\"Invia\">";
    inputs += "</form>";
    return inputs;
  }
  return String();
}

void setup()
{
  //Inizializzo le due porte seriali
  Serial.begin(115200);
  Serial.println("Il progetto si sta inizializzando");
  Serial1.begin(115200, SERIAL_8N1, CONFIG_MB_UART_RXD, CONFIG_MB_UART_TXD); //Configurazione custom

  //Inizializzo canale di comunicazione su seriale RS485
  pinMode(MAX485_DE_RE, OUTPUT);
  digitalWrite(MAX485_DE_RE, LOW);

  // Inizializzo SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //Inizializzo connessione wifi con esp32
  Serial.print("Connecting to ");
  Serial.println(SSID_NAME);
  WiFi.begin(SSID_NAME, SSID_PASSWORD); //Credenziali prese dal file arduino_secrets.h
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Stampo l'ip address e inizializzo il WiFi Server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", String(), false, processor); });
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/style.css", "text/css"); });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String inputMessage1;
              String inputMessage2;
              // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
              if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2))
              {
                inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
                inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
                writeDigital(inputMessage1.toInt() + 303, 1, inputMessage2.toInt());
              }
              else
              {
                inputMessage1 = "No message sent";
                inputMessage2 = "No message sent";
              }
              Serial.print("Relay: ");
              Serial.print(inputMessage1);
              Serial.print(" - Set to: ");
              Serial.println(inputMessage2);
              request->send(SPIFFS, "/index.html", String(), false, processor);
            });
  server.on("/input", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/input.html", String(), false, processor); });
  server.on(
      "/input", HTTP_POST, [](AsyncWebServerRequest *request)
      {
        int params = request->params();
        for (int i = 0; i < params; i++)
        {
          AsyncWebParameter *p = request->getParam(i);
          if (p->isPost())
          {
            //Serial.printf("%s: %s \n", p->name().c_str(), p->value().c_str());
            nomiCampi[i] = p->value();
            Serial.println(nomiCampi[i]);
          }
        }
        request->send(200, "text/html", "HTTP POST request sent to your ESP<br><a href=\"/\">Ritorna alla home page</a>");
      });

  server.begin();

  //Inizializzo il modbus
  master.begin(10, Serial1);
  master.preTransmission(preTransmission);
  master.postTransmission(postTransmission);

  delay(1000);

  Serial.println("Progetto inizializzato");
}

void loop()
{
  // put your main code here, to run repeatedly:
}
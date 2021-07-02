#include <Arduino.h>
#include "arduino_secrets.h"
#include <ModbusMaster.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "SPIFFS.h"
#include <EEPROM.h>

#include "trackle_esp32_wifi.h"

#include <mutex>

//Pin per la comunicazione RS485
#define MAX485_DE_RE 14
#define CONFIG_MB_UART_TXD 4
#define CONFIG_MB_UART_RXD 36

//Inizializzo la grandezza della EEPROM
#define EEPROM_SIZE 2048

const bool debug = true;

void printDebug(String msg)
{
  if (debug)
  {
    Serial.println(msg);
  }
}

//Variabili per il modbus
uint8_t digitalOutput[4]; //digitalOutput
int digitalInput[4];      //digitalInput
uint8_t analogOutput[4];  //analogOutput
float analogInput[4];     //analogInput
float sonda[4];           //sonde di temperatura

// Inizalizzo la libreia ModBus
ModbusMaster master;

/**
 * Timeout per lettura e scrittura sui registri della scheda
 * Serve anche per aggiurnare i dati presenti nel json
 */
uint32_t msModbus;

//Mutex
std::mutex modbus_mutex;

//Funzioni per il MODBUS
void preTransmission() //Questa viene eseguida prima di aprire la comunicazione rs485
{
  digitalWrite(MAX485_DE_RE, HIGH);
  delayMicroseconds(70);
}

void postTransmission() //Questa viene eseguita dopo la chiusura della comunicazione rs485
{
  Serial1.flush();
  delayMicroseconds(1562);
  digitalWrite(MAX485_DE_RE, LOW);
}

//Funzioni per il modbus

/**
 * Questa Funzione serve per leggere tutti i registri della scheda
 * Sia gli analogici che i digitali (26 registri)
 */
void readModbus()
{
  uint8_t result;

  delay(100);
  result = master.readHoldingRegisters(304, 26);

  if (result == master.ku8MBSuccess)
  {
    digitalOutput[0] = master.getResponseBuffer(0);
    digitalOutput[1] = master.getResponseBuffer(1);
    digitalOutput[2] = master.getResponseBuffer(2);
    digitalOutput[3] = master.getResponseBuffer(3);
    analogOutput[0] = master.getResponseBuffer(4);
    analogOutput[1] = master.getResponseBuffer(5);
    analogOutput[2] = master.getResponseBuffer(6);
    analogOutput[3] = master.getResponseBuffer(7);
    digitalInput[0] = master.getResponseBuffer(14);
    digitalInput[1] = master.getResponseBuffer(15);
    digitalInput[2] = master.getResponseBuffer(15);
    digitalInput[3] = master.getResponseBuffer(17);
    analogInput[0] = master.getResponseBuffer(18) / 10240.000 * 10;
    analogInput[1] = master.getResponseBuffer(19) / 10240.000 * 10;
    analogInput[2] = master.getResponseBuffer(20) / 10240.000 * 10;
    analogInput[3] = master.getResponseBuffer(21) / 10240.000 * 10;
    sonda[0] = master.getResponseBuffer(22) / 100.00;
    sonda[1] = master.getResponseBuffer(23) / 100.00;
    sonda[2] = master.getResponseBuffer(24) / 100.00;
    sonda[3] = master.getResponseBuffer(25) / 100.00;
  }
}
/**
 * @brief Funzione per scrivere su tutti i registri
 */
void writeModbus()
{
  for (int i = 0; i < 4; i++)
  {
    master.setTransmitBuffer(i, digitalOutput[i]);
    master.setTransmitBuffer(i + 4, analogOutput[i]);
  }
  master.writeMultipleRegisters(304, 8);
}

//Struct EEPROM output analogici
struct pinOut
{
  uint8_t analogOutput1;
  uint8_t analogOutput2;
  uint8_t analogOutput3;
  uint8_t analogOutput4;
  uint8_t digitalOutput1;
  uint8_t digitalOutput2;
  uint8_t digitalOutput3;
  uint8_t digitalOutput4;
} eepromOutput;

//Struct EEPROM nomi campi (lunghezza massima di 20 caratteri)
struct campi
{
  char digitalOutput1[20];
  char digitalOutput2[20];
  char digitalOutput3[20];
  char digitalOutput4[20];
  char analogOutput1[20];
  char analogOutput2[20];
  char analogOutput3[20];
  char analogOutput4[20];
  char digitalInput1[20];
  char digitalInput2[20];
  char digitalInput3[20];
  char digitalInput4[20];
  char analogInput1[20];
  char analogInput2[20];
  char analogInput3[20];
  char analogInput4[20];
  char sonda1[20];
  char sonda2[20];
  char sonda3[20];
  char sonda4[20];
} nomi;

//MAC address del pc
byte _mac[] = {0x00, 0x08, 0xDC, 0x11, 0x27, 0x8C};
// Creo il server AsyncWebServer sulla porta 80
AsyncWebServer server2(80);

//Variabili per accensione e spegnimento relay
const char *PARAM_INPUT_1 = "output"; //id del relay associato allo switch
const char *PARAM_INPUT_2 = "state";  //Stato, se acceso oppure spento

//Variabile per slider
const char *PARAM_SLIDER_1 = "id";    //id dell'analogOutput collegato allo slider
const char *PARAM_SLIDER_2 = "value"; //Valore dello slider

String outputState(int output)
{
  if (digitalOutput[output] == 1)
  {
    return "checked"; //Quando il relay è acceso
  }
  else
  {
    return ""; // Quando è spento
  }
}

// Funzione che serve per sostiuire i placeholder nella pagina HTML con il codice scritto sotto
String processor(const String &var)
{
  //Intestazione Relay
  if (var == "RELAY1") //Intestazione Relay1
  {
    return String(nomi.digitalOutput1).length() == 0 ? "Relay1" : nomi.digitalOutput1;
  }
  else if (var == "RELAY2") //Intestazione Relay2
  {
    return String(nomi.digitalOutput2).length() == 0 ? "Relay2" : nomi.digitalOutput2;
  }
  else if (var == "RELAY3") //Intestazione Relay3
  {
    return String(nomi.digitalOutput3).length() == 0 ? "Relay3" : nomi.digitalOutput3;
  }
  else if (var == "RELAY4") //Intestazione Relay4
  {
    return String(nomi.digitalOutput4).length() == 0 ? "Relay4" : nomi.digitalOutput4;
  }

  //Intestazione DigitalInput
  else if (var == "DIGITALINPUT1") //Intestazione DigitalInput1
  {
    return String(nomi.digitalInput1).length() == 0 ? "DigitalInput1" : nomi.digitalInput1;
  }
  else if (var == "DIGITALINPUT2") //Intestazione DigitalInput2
  {
    return String(nomi.digitalInput2).length() == 0 ? "DigitalInput2" : nomi.digitalInput2;
  }
  else if (var == "DIGITALINPUT3") //Intestazione DIgitalInput3
  {
    return String(nomi.digitalInput3).length() == 0 ? "DigitalInput3" : nomi.digitalInput3;
  }
  else if (var == "DIGITALINPUT4") //Intestazione DigitalInput4
  {
    return String(nomi.digitalInput4).length() == 0 ? "DigitalInput4" : nomi.digitalInput4;
  }

  //Intestazione AnalogOutput
  else if (var == "ANALOGOUTPUT1") //Intestazione AnalogOutput1
  {
    return String(nomi.analogOutput1).length() == 0 ? "AnalogOutput1" : nomi.analogOutput1;
  }
  else if (var == "ANALOGOUTPUT2") //Intestazione AnalogOutput2
  {
    return String(nomi.analogOutput2).length() == 0 ? "AnalogOutput2" : nomi.analogOutput2;
  }
  else if (var == "ANALOGOUTPUT3") //Intestazione AnalogOutput3
  {
    return String(nomi.analogOutput3).length() == 0 ? "AnalogOutput3" : nomi.analogOutput3;
  }
  else if (var == "ANALOGOUTPUT4") //Intestazione AnalogOutput4
  {
    return String(nomi.analogOutput4).length() == 0 ? "AnalogOutput4" : nomi.analogOutput4;
  }

  //Intestazione AnalogInput
  else if (var == "ANALOGINPUT1") //Intestazione AnalogInput1
  {
    return String(nomi.analogInput1).length() == 0 ? "AnalogInput1" : nomi.analogInput1;
  }
  else if (var == "ANALOGINPUT2") //Intestazione AnalogInput1
  {
    return String(nomi.analogInput2).length() == 0 ? "AnalogInput2" : nomi.analogInput2;
  }
  else if (var == "ANALOGINPUT3") //Intestazione AnalogInput3
  {
    return String(nomi.analogInput3).length() == 0 ? "AnalogInput3" : nomi.analogInput3;
  }
  else if (var == "ANALOGINPUT4") //Intestazione AnalogInput4
  {
    return String(nomi.analogInput4).length() == 0 ? "AnalogInput4" : nomi.analogInput4;
  }

  //Intestazione Sonde
  else if (var == "SONDA1") //Intestazione Sonda1
  {
    return String(nomi.sonda1).length() == 0 ? "Sonda1" : nomi.sonda1;
  }
  else if (var == "SONDA2") //Intestazione Sonda2
  {
    return String(nomi.sonda2).length() == 0 ? "Sonda1" : nomi.sonda2;
  }
  else if (var == "SONDA3") //Intestazione Sonda3
  {
    return String(nomi.sonda3).length() == 0 ? "Sonda1" : nomi.sonda3;
  }
  else if (var == "SONDA4") //Intestazione Sonda4
  {
    return String(nomi.sonda4).length() == 0 ? "Sonda1" : nomi.sonda4;
  }

  //Input nella pagina di inserimento
  else if (var == "NOMERELAY1") //Nome Relay1
  {
    return nomi.digitalOutput1;
  }
  else if (var == "NOMERELAY2") //Nome Relay2
  {
    return nomi.digitalOutput2;
  }
  else if (var == "NOMERELAY3") //Nome Relay3
  {
    return nomi.digitalOutput3;
  }
  else if (var == "NOMERELAY4") //Nome Relay4
  {
    return nomi.digitalOutput4;
  }
  else if (var == "NOMEANALOGOUTPUT1") //Nome AnalogOutput1
  {
    return nomi.analogOutput1;
  }
  else if (var == "NOMEANALOGOUTPUT2") //Nome AnalogOutput2
  {
    return nomi.analogOutput2;
  }
  else if (var == "NOMEANALOGOUTPUT3") //Nome AnalogOutput3
  {
    return nomi.analogOutput3;
  }
  else if (var == "NOMEANALOGOUTPUT4") //Nome AnalogOutput4
  {
    return nomi.analogOutput4;
  }
  else if (var == "NOMEDIGITALINPUT1") //Nome DigitalInput1
  {
    return nomi.digitalInput1;
  }
  else if (var == "NOMEDIGITALINPUT2") //Nome DigitalInput2
  {
    return nomi.digitalInput2;
  }
  else if (var == "NOMEDIGITALINPUT3") //Nome DigitalInput3
  {
    return nomi.digitalInput3;
  }
  else if (var == "NOMEDIGITALINPUT4") //Nome DigitalInput4
  {
    return nomi.digitalInput4;
  }
  else if (var == "NOMEANALOGINPUT1") //Nome AnalogInput1
  {
    return nomi.analogInput1;
  }
  else if (var == "NOMEANALOGINPUT2") //Nome AnalogInput2
  {
    return nomi.analogInput2;
  }
  else if (var == "NOMEANALOGINPUT3") //Nome AnalogInput3
  {
    return nomi.analogInput3;
  }
  else if (var == "NOMEANALOGINPUT4") //Nome AnalogInput4
  {
    return nomi.analogInput4;
  }
  else if (var == "NOMESONDA1") //Nome Sonda1
  {
    return nomi.sonda1;
  }
  else if (var == "NOMESONDA2") //Nome Sonda2
  {
    return nomi.sonda2;
  }
  else if (var == "NOMESONDA3") //Nome Sonda3
  {
    return nomi.sonda3;
  }
  else if (var == "NOMESONDA4") //Nome Sonda4
  {
    return nomi.sonda4;
  }

  return String();
}

//Json dove salvo i valori dei registri
std::string jsonValori;

//Funzione per aggiornare i dati del json
std::string updateJson()
{
  jsonValori = ("{\"digitalOutput1\":");
  jsonValori.append(String(digitalOutput[0]).c_str());
  jsonValori.append(",\"digitalOutput2\":");
  jsonValori.append(String(digitalOutput[1]).c_str());
  jsonValori.append(",\"digitalOutput3\":");
  jsonValori.append(String(digitalOutput[2]).c_str());
  jsonValori.append(",\"digitalOutput4\":");
  jsonValori.append(String(digitalOutput[3]).c_str());

  jsonValori.append(",\"analogOutput1\":");
  jsonValori.append(String(analogOutput[0]).c_str());
  jsonValori.append(",\"analogOutput2\":");
  jsonValori.append(String(analogOutput[1]).c_str());
  jsonValori.append(",\"analogOutput3\":");
  jsonValori.append(String(analogOutput[2]).c_str());
  jsonValori.append(",\"analogOutput4\":");
  jsonValori.append(String(analogOutput[3]).c_str());

  jsonValori.append(",\"digitalInput1\":");
  jsonValori.append(String(digitalInput[0]).c_str());
  jsonValori.append(",\"digitalInput2\":");
  jsonValori.append(String(digitalInput[1]).c_str());
  jsonValori.append(",\"digitalInput3\":");
  jsonValori.append(String(digitalInput[2]).c_str());
  jsonValori.append(",\"digitalInput4\":");
  jsonValori.append(String(digitalInput[3]).c_str());

  jsonValori.append(",\"analogInput1\":");
  jsonValori.append(String(analogInput[0]).c_str());
  jsonValori.append(",\"analogInput2\":");
  jsonValori.append(String(analogInput[1]).c_str());
  jsonValori.append(",\"analogInput3\":");
  jsonValori.append(String(analogInput[2]).c_str());
  jsonValori.append(",\"analogInput4\":");
  jsonValori.append(String(analogInput[3]).c_str());

  jsonValori.append(",\"sonda1\":");
  jsonValori.append(String(sonda[0]).c_str());
  jsonValori.append(",\"sonda2\":");
  jsonValori.append(String(sonda[1]).c_str());
  jsonValori.append(",\"sonda3\":");
  jsonValori.append(String(sonda[2]).c_str());
  jsonValori.append(",\"sonda4\":");
  jsonValori.append(String(sonda[3]).c_str());
  jsonValori.append("}");
  return jsonValori;
}

//Variabili trackle
const char *privatekey = "-----BEGIN EC PRIVATE KEY-----\nMHcCAQEEIKKrJwZE/cbjFPZ7906QPsXWuvxFEXpfm6qN9BxUeFjMoAoGCCqGSM49\nAwEHoUQDQgAEaeNexwuMTbHLg4fyFVSGYnj1KCm35sPyfFHCgpqoaG9NQ88ADJ+y\nsjAIl0qjq5ChImjNBRkKEY8VT+Hl9LNweA==\n-----END EC PRIVATE KEY-----\n";
const char *deviceid = "10af3922e9c583002809960c";

//Funzione cloud per l'accensione e lo spegnimento dei relay
int cloudWriteDigital(const char *param)
{
  if (strlen(param) < 3)
  {
    return -1;
  }
  char value[3];
  strcpy(value, param);
  char *reg;
  char *num;
  reg = strtok(value, ",");
  printDebug("Registro = " + String(atoi(reg) + 303));
  if (atoi(reg) < 1 || atoi(reg) > 4 || isalpha(*reg))
  {
    return -1;
  }
  num = strtok(NULL, ",");
  printDebug("Valore = " + String(atoi(num)));
  if (atoi(num) < 0 || atoi(num) > 1 || isalpha(*num))
  {
    return -1;
  }
  digitalOutput[atoi(reg) - 1] = atoi(num);
  return 1;
}

//Funzione Cloud per la modifica dei valori degli output analogici
int cloudWriteAnalog(const char *param)
{
  if (strlen(param) < 3)
  {
    return -1;
  }
  char value[5];
  strcpy(value, param);
  char *reg;
  char *num;
  reg = strtok(value, ",");
  printDebug(String(atoi(reg) + 307));
  if (atoi(reg) < 1 || atoi(reg) > 4 || isalpha(*reg))
  {
    return -1;
  }
  num = strtok(NULL, ",");
  printDebug(String(atoi(num)));
  if (atoi(num) < 0 || atoi(num) > 255 || isalpha(*num))
  {
    return -1;
  }
  analogOutput[atoi(reg) - 1] = atoi(num);
  return 1;
}

int cloudWriteAllDigital(const char *param)
{
  if (atoi(param) < 0 || atoi(param) > 1 || isalpha(*param))
  {
    return -1;
  }

  for (int i = 0; i < 4; i++)
  {
    digitalOutput[i] = atoi(param);
  }

  return 1;
}

void setup()
{
  //Inizializzo le due porte seriali
  Serial.begin(115200);
  printDebug("Inizalizzazione progetto");
  Serial1.begin(115200, SERIAL_8N1, CONFIG_MB_UART_RXD, CONFIG_MB_UART_TXD); //Configurazione custom

  //Inizializzo canale di comunicazione su seriale RS485
  pinMode(MAX485_DE_RE, OUTPUT);
  digitalWrite(MAX485_DE_RE, LOW);

  //Inizializzo la EEPROM
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(1, eepromOutput); //leggere l'ultimo stato degli output dalla memoria flash
  analogOutput[0] = eepromOutput.analogOutput1;
  analogOutput[1] = eepromOutput.analogOutput2;
  analogOutput[2] = eepromOutput.analogOutput3;
  analogOutput[3] = eepromOutput.analogOutput4;
  digitalOutput[0] = eepromOutput.digitalOutput1;
  digitalOutput[1] = eepromOutput.digitalOutput2;
  digitalOutput[2] = eepromOutput.digitalOutput3;
  digitalOutput[3] = eepromOutput.digitalOutput4;

  EEPROM.get(30, nomi); //leggere l'ultimo stato dei nomi dalla memoria flash

  // Inizializzo SPIFFS
  if (!SPIFFS.begin(true))
  {
    printDebug("Errore durante l'inizializzazione della SPIFFS");
    return;
  }

  //Inizializzo connessione wifi con esp32
  printDebug("Connettendo a: " + String(SSID_NAME));

  //Prima begin
  trackle.begin(DEVICEID, PRIVATEKEY);
  //Dopo funzioni cloud
  trackle.post("cloudWriteDigital", cloudWriteDigital);
  trackle.post("cloudWriteAnalog", cloudWriteAnalog);
  trackle.post("cloudWriteAllDigital", cloudWriteAllDigital);
  trackle.get("json", jsonValori);
  //Connessione in cloud
  trackle.connect(SSID_NAME, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    printDebug(".");
  }
  // Stampo l'ip address e inizializzo il WiFi Server
  printDebug("WiFi connesso.");
  printDebug("IP address: " + WiFi.localIP().toString());

  // Route per / pagina web, homepage dove sono contenuti tutte le funzionalità del WebServer
  server2.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
             { request->send(SPIFFS, "/index.html", String(), false, processor); });
  // Route per caricare lo stile della pagina
  server2.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
             { request->send(SPIFFS, "/style.css", "text/css"); });
  //Funzione per l'accensione e spegnimento dei relay
  server2.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
             {
               String inputMessage1; //Variabile per salvare id dello switch
               // GET Relays value
               if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2))
               {
                 inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
                 printDebug("Id dello switch = " + inputMessage1);
                 inputMessage1 = inputMessage1.substring(5, 6);
                 modbus_mutex.lock();
                 for (int i = 0; i < 4; i++)
                 {
                   if (inputMessage1.toInt() == i + 1)
                   {
                     printDebug("Prima = " + String(digitalOutput[i]));
                     digitalOutput[i] = request->getParam(PARAM_INPUT_2)->value().toInt(); //Salvo il valore nell'array digitalOutput[4]
                     printDebug("Dopo = " + String(digitalOutput[i]));
                   }
                 }
                 modbus_mutex.unlock();
               }
               else
               {
                 inputMessage1 = "Nessun messaggio inviato";
               }
               request->send(200); //Invio una risposta al server
             });
  //Funzione per la scrittura su output analogici tramite gli slider
  server2.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request)
             {
               String inputMessage1; //Variabile per salvare id dello slider

               if (request->hasParam(PARAM_SLIDER_1) && request->hasParam(PARAM_SLIDER_2)) //Controllo se questi due parametri sono presenti
               {
                 inputMessage1 = request->getParam(PARAM_SLIDER_1)->value().substring(4, 5); //Varibile per salvare l'id dello slider
                 modbus_mutex.lock();
                 for (int i = 0; i < 4; i++)
                 {
                   //In base all'id dello slider scrivo sul registro corretto
                   if (inputMessage1.toInt() == i + 1)
                   {
                     printDebug("Prima = " + String(analogOutput[i]));
                     analogOutput[i] = request->getParam(PARAM_SLIDER_2)->value().toInt(); //Salvo il valore nell'array analogOutput[4]
                     printDebug("Dopo = " + String(analogOutput[i]));
                   }
                 }
                 modbus_mutex.unlock();
               }
               else
               {
                 inputMessage1 = "Nessun messaggio inviato";
               }
               request->send(200); //Invio una risposta al server
             });
  //Invio un file json con lo stato dei digitalInput e il valore degli analogInput
  server2.on("/json", HTTP_GET, [](AsyncWebServerRequest *request)
             {
               updateJson();
               request->send(200, "text/plain", jsonValori.c_str());
             });
  //Route per la pagina /inserimento, che serve per aprire la form
  server2.on("/inserimento", HTTP_GET, [](AsyncWebServerRequest *request)
             { request->send(SPIFFS, "/input.html", String(), false, processor); });
  //Funzione per recuperare i valori inseriti all'interno della form
  server2.on(
      "/inserimento", HTTP_POST, [](AsyncWebServerRequest *request)
      {
        if (request->hasParam("relay1", true) && request->hasParam("relay2", true) && request->hasParam("relay3", true) && request->hasParam("relay4", true))
        {
          strcpy(nomi.digitalOutput1, request->getParam("relay1", true)->value().c_str());
          strcpy(nomi.digitalOutput2, request->getParam("relay2", true)->value().c_str());
          strcpy(nomi.digitalOutput3, request->getParam("relay3", true)->value().c_str());
          strcpy(nomi.digitalOutput4, request->getParam("relay4", true)->value().c_str());
        }
        if (request->hasParam("aOut1", true) && request->hasParam("aOut2", true) && request->hasParam("aOut3", true) && request->hasParam("aOut4", true))
        {
          strcpy(nomi.analogOutput1, request->getParam("aOut1", true)->value().c_str());
          strcpy(nomi.analogOutput2, request->getParam("aOut2", true)->value().c_str());
          strcpy(nomi.analogOutput3, request->getParam("aOut3", true)->value().c_str());
          strcpy(nomi.analogOutput4, request->getParam("aOut4", true)->value().c_str());
        }
        if (request->hasParam("dIn1", true) && request->hasParam("dIn2", true) && request->hasParam("dIn3", true) && request->hasParam("dIn4", true))
        {
          strcpy(nomi.digitalInput1, request->getParam("dIn1", true)->value().c_str());
          strcpy(nomi.digitalInput2, request->getParam("dIn2", true)->value().c_str());
          strcpy(nomi.digitalInput3, request->getParam("dIn3", true)->value().c_str());
          strcpy(nomi.digitalInput4, request->getParam("dIn4", true)->value().c_str());
        }
        if (request->hasParam("aIn1", true) && request->hasParam("aIn2", true) && request->hasParam("aIn3", true) && request->hasParam("aIn4", true))
        {
          strcpy(nomi.analogInput1, request->getParam("aIn1", true)->value().c_str());
          strcpy(nomi.analogInput2, request->getParam("aIn2", true)->value().c_str());
          strcpy(nomi.analogInput3, request->getParam("aIn3", true)->value().c_str());
          strcpy(nomi.analogInput4, request->getParam("aIn4", true)->value().c_str());
        }
        if (request->hasParam("sonda1", true) && request->hasParam("sonda2", true) && request->hasParam("sonda3", true) && request->hasParam("sonda4", true))
        {
          strcpy(nomi.sonda1, request->getParam("sonda1", true)->value().c_str());
          strcpy(nomi.sonda2, request->getParam("sonda2", true)->value().c_str());
          strcpy(nomi.sonda3, request->getParam("sonda3", true)->value().c_str());
          strcpy(nomi.sonda4, request->getParam("sonda4", true)->value().c_str());
        }
        //Scrivo in EEPROM
        EEPROM.put(30, nomi);
        //Salvo tutto quello scritto nell'EEPROM
        printDebug("Salvo in EEPROM");
        //Invio le modifiche alla EEPROM
        EEPROM.commit();
        //Quando la richiesta va a buon fine, verrà mostrata questa pagina
        request->send(SPIFFS, "/post.html", String(), false, processor);
      });

  server2.begin();

  //Inizializzo il modbus
  master.begin(10, Serial1);
  master.preTransmission(preTransmission);
  master.postTransmission(postTransmission);

  printDebug("Progetto inizializzato");
}

void loop()
{
  //Leggo e scrivo tutti i registri modbus, aggiorno il file json
  if (millis() - msModbus >= 500)
  {
    modbus_mutex.lock();   //chiudo se non è disponibile
    writeModbus();         //Scrivo sui registri
    readModbus();          //Leggo i registri
    modbus_mutex.unlock(); //riapro quando ritorna disponibile

    updateJson(); //Aggiorno il file Json

    msModbus = millis();
  }

  //Avvio trackle
  trackle.loop();

  //Scrivo nell'EEPROM i valori dei pin di Output (Analogici e Digitali)
  if (eepromOutput.analogOutput1 != analogOutput[0] || eepromOutput.analogOutput2 != analogOutput[1] || eepromOutput.analogOutput3 != analogOutput[2] || eepromOutput.analogOutput4 != analogOutput[3] || eepromOutput.digitalOutput1 != digitalOutput[0] || eepromOutput.digitalOutput2 != digitalOutput[1] || eepromOutput.digitalOutput3 != digitalOutput[2] || eepromOutput.digitalOutput4 != digitalOutput[3])
  {
    printDebug("Valore eeprom digitalOutput1 prima = " + String(eepromOutput.digitalOutput1));
    printDebug("Valore eeprom analogOutput1 prima = " + String(eepromOutput.analogOutput1));
    eepromOutput = {
        analogOutput[0], analogOutput[1], analogOutput[2], analogOutput[3],
        digitalOutput[0], digitalOutput[1], digitalOutput[2], digitalOutput[3]};
    printDebug("Salvo in EEPROM");
    printDebug("Valore eeprom digitalOutput1 dopo = " + String(eepromOutput.digitalOutput1));
    printDebug("Valore eeprom analogOutput1 dopo = " + String(eepromOutput.analogOutput1));
    EEPROM.put(1, eepromOutput);
    EEPROM.commit();
  }
}
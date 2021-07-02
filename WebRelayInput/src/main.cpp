#include <Arduino.h>

#include "KMPProDinoESP32.h"
#include "KMPCommon.h"
#include "arduino_secrets.h"

#include <WiFi.h>
#include <WiFiClient.h>

// if this define is uncommented example supports both Ethernet and WiFi
//#define ETH_TEST

byte _mac[] = { 0x00, 0x08, 0xDC, 0x11, 0x27, 0x8C }; 
// The IP address will be dependent on your local network.
IPAddress _ip(192, 168, 1, 197);
// Local port. The port 80 is default for HTTP
const uint16_t LOCAL_PORT = 80;
// Initialize the Ethernet server library.
EthernetServer _ethServer(LOCAL_PORT);
WiFiServer _wifiServer(LOCAL_PORT);

// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 500ms = 0.5s)
const long timeoutTime = 500;

/**
 * @brief Setup void. Ii is Arduino executed first. Initialize DiNo board.
 *
 * @return void
 */
void setup(void)
{
	delay(5000);
	Serial.begin(115200);
	Serial.println("The example is starting...");

	KMPProDinoESP32.begin(ProDino_ESP32);
	//KMPProDinoESP32.begin(ProDino_ESP32_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa_RFM);
#ifdef ETH_TEST
	KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);
#endif // ETH_TEST

	KMPProDinoESP32.setStatusLed(blue);

	// Connect to WiFi network
	WiFi.begin(SSID, SSID_PASSWORD);
	Serial.print("\n\r \n\rWiFi is connecting");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	
	Serial.println();
	Serial.print("WiFi IP: ");
	Serial.print(WiFi.localIP());
	Serial.print(":");
	Serial.println(LOCAL_PORT);

	_wifiServer.begin();

#ifdef ETH_TEST
	// Start the Ethernet connection and the server.
	// Using static IP address
	//Ethernet.begin(_mac, _ip);
	// Getting IP from DHCP
	if (Ethernet.begin(_mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		while (1);
	}
	_ethServer.begin();

	Serial.println();
	Serial.print("Ethernet IP: ");
	Serial.print(Ethernet.localIP());
	Serial.print(":");
	Serial.println(LOCAL_PORT);
#endif // ETH_TEST

	KMPProDinoESP32.offStatusLed();

	Serial.println("The example is started.");
}

/**
 * @brief Build HTML page.
 *
 * @return void
 */
String BuildPage()
{
	//Input
	String tableBody = "";
	String tableHeader = "";

	for (uint8_t i = 0; i < OPTOIN_COUNT; i++)
	{
		String inputNumber = String(i + 1);
		tableHeader += "<th>In " + inputNumber + "</th>";

		char* cellColor;
		char* cellStatus;
		if (KMPProDinoESP32.getOptoInState(i))
		{
			cellColor = (char*)W_RED;
			cellStatus = (char*)W_ON;
		}
		else
		{
			cellColor = (char*)W_GREEN;
			cellStatus = (char*)W_OFF;
		}

		tableBody += "<td bgcolor='" + String(cellColor) + "'>" + String(cellStatus) + "</td>";
	}

	String inputs = "<thead><tr>" + tableHeader + "</tr></thead>"
		+ "<tbody><tr>" + tableBody + "</tr></tbody>";

	//Relay
	String rows = "";
	for (uint8_t i = 0; i < RELAY_COUNT; i++)
	{
		// Row i, cell 1
		String relayNumber = String(i + 1);
		rows += "<tr><td>Relay " + relayNumber + "</td>";

		char* cellColor;
		char* cellStatus;
		char* nextRelayStatus;
		if (KMPProDinoESP32.getRelayState(i))
		{
			cellColor = (char*)W_RED;
			cellStatus = (char*)W_ON;
			nextRelayStatus = (char*)W_OFF;
		}
		else
		{
			cellColor = (char*)W_GREEN;
			cellStatus = (char*)W_OFF;
			nextRelayStatus = (char*)W_ON;
		}

		// Cell i,2
		rows += "<td bgcolor='" + String(cellColor) + "'>" + String(cellStatus) + "</td>";

		// Cell i,3
		rows += "<td><input type='submit' name='r" + String(relayNumber) + "' value='" + String(nextRelayStatus) + "'/ ></td></tr>";
	}

	return "<html><head><title>" + String(KMP_ELECTRONICS_LTD) + " " + String(PRODINO_ESP32) + " - Web Relay</title></head>\
		<body><div style='text-align: center'>\
		<br><hr />\
		<h1 style = 'color: #0066FF;'>" + String(PRODINO_ESP32) + " - Relay example</h1>\
		<hr /><br><br>\
		<form method = 'post'>\
		<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ rows
		+ "</table></form><br><br><hr />\
		<h1 style = 'color: #0066FF;'>" + String(PRODINO_ESP32) + " - Inputs example</h1>\
		<hr /><br><br>\
		<table border='1' width='300' cellpadding='5' cellspacing='0' align='center' style='text-align:center; font-size:large; font-family:Arial,Helvetica,sans-serif;'>"
		+ inputs
		+ "</table><br><br><hr /></div></body></html>";
}

/**
* @brief ReadRelayRequest void. Read and parse client request.
* 	First row of client request is similar to:
*		GET / HTTP/1.1
* You can check communication client-server get program Smart Sniffer: http://www.nirsoft.net/utils/smsniff.html
*
* @return bool Returns true if the request was expected.
*/
bool ReadRelayRequest(Stream* client)
{
	// Loop while read all request.
	// Loop while read all request.
	// Read first and last row from request.
	String firstRow;
	String lastRow;
	if (ReadHttpRequestLine(client, &firstRow))
	{
		while (ReadHttpRequestLine(client, &lastRow));
	}

	if (GetRequestType(firstRow.c_str()) == GET)
	{
		return true;
	}

	// Invalid request type.
	if (GetRequestType(firstRow.c_str()) != POST || lastRow.length() == 0)
	{
		Serial.println(">> Invalid request type.");
		return false;
	}

	// Relay request.
	if (lastRow[0] == 'r')
	{
		// From POST parameters we get relay number and new status.
		uint8_t relay = CharToInt(lastRow[1]) - 1;
		bool newState = lastRow.endsWith(W_ON);

		KMPProDinoESP32.setRelayState(relay, newState);
	}

	Serial.println(">> End client request.");
	return true;
}

/**
* @brief WriteRelayResponse void. Write html response.
*
*
* @return void
*/
void WriteRelayResponse(Client* client)
{
	if (!client->connected())
	{
		return;
	}

	// Response write.
	// Send a standard http header.
	client->print(HEADER_200_TEXT_HTML);

	// Add web page HTML.
	String content = BuildPage();
	client->print(content.c_str());
}

bool ReadInputRequest(Stream* client)
{
	// Loop while read all request.
	// Loop while read all request.
	// Read first and last row from request.
	String firstRow;
	String lastRow;
	if (ReadHttpRequestLine(client, &firstRow))
	{
		while (ReadHttpRequestLine(client, &lastRow));
	}

	if (GetRequestType(firstRow.c_str()) == GET)
	{
		return true;
	}
	else
	{
		Serial.println(">> Invalid request type.");
		return false;
	}
}

void WriteInputResponse(Client* client)
{
	if (!client->connected())
	{
		return;
	}

	// Response write.
	// Send a standard http header.
	client->print(HEADER_200_TEXT_HTML);

	// Add web page HTML.
	String content = BuildPage();
	client->print(content.c_str());
}

/**
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop()
{
	KMPProDinoESP32.processStatusLed(green, 1000);

	Client* client = NULL;
	// Check if a client has connected
	WiFiClient wifiClient = _wifiServer.available();
	if (wifiClient)
	{
		KMPProDinoESP32.setStatusLed(yellow);

		Serial.println("-- wifiClient --");
		// Wait for data from the client that become available
		previousTime = millis();
		while (wifiClient.connected() && !wifiClient.available()) {
			delay(1);
			if(millis() - previousTime >= timeoutTime){
				break;
			}
		}

		client = &wifiClient;
	}
#ifdef ETH_TEST
	EthernetClient ethClient = _ethServer.available();
	if (ethClient && ethClient.connected())
	{
		Serial.println("-- ethClient --");
		client = &ethClient;
	}
#endif
	if (client == NULL)
	{
		return;
	}

	KMPProDinoESP32.setStatusLed(yellow);

	Serial.println(">> Client connected.");

	// Read client relay request.
	if (ReadRelayRequest(client))
	{
		WriteRelayResponse(client);
	}

	// Read client input request.
	if (ReadRelayRequest(client))
	{
		WriteRelayResponse(client);
	}

	// Close the client connection.
	client->stop();
	Serial.println(">> Client disconnected.");
	Serial.println();

	// If client disconnected switch Off status led.
	KMPProDinoESP32.offStatusLed();
}
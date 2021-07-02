#include <Arduino.h>
#include "KMPProDinoESP32.h"
#include "KMPCommon.h"

const int CMD_PREFFIX_LEN = 3;
const char CMD_PREFFIX[CMD_PREFFIX_LEN + 1] = "FFR";

const uint8_t BUFF_MAX = 16;

char _dataBuffer[BUFF_MAX];
char _resultBuffer[BUFF_MAX];

/**
* @brief Setup void. Ii is Arduino executed first. Initialize DiNo board.
*
* @return void
*/
void setup()
{
	delay(5000);
	Serial.begin(115200);
	Serial.println("The example RS485Relay is starting...");

	KMPProDinoESP32.begin(ProDino_ESP32);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa_RFM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);
	KMPProDinoESP32.setStatusLed(blue);

	// Start RS485 with baud 19200 and 8N1.
	KMPProDinoESP32.rs485Begin(9600);
	Serial.println("The example RS485Relay is started.");
	delay(1000);

	KMPProDinoESP32.offStatusLed();
}

void ProcessData()
{
	int len = strlen(_dataBuffer);

	// Validate input data.
	if (len < CMD_PREFFIX_LEN || !startsWith(_dataBuffer, CMD_PREFFIX))
	{
		Serial.print("Command is not valid.");
		return;
	}

	// Command set relay status FFRxxxx
	if (len == CMD_PREFFIX_LEN + RELAY_COUNT)
	{
		int relayNum = 0;
		for (int i = CMD_PREFFIX_LEN; i < CMD_PREFFIX_LEN + RELAY_COUNT; i++)
		{
			// Set relay status if only chars are 0 or 1.
			if (_dataBuffer[i] == CH_0 || _dataBuffer[i] == CH_1)
			{
				KMPProDinoESP32.setRelayState(relayNum, _dataBuffer[i] == CH_1);
			}

			++relayNum;
		}
	}

	// Prepare relays statuses.
	strcpy(_resultBuffer, CMD_PREFFIX);
	int relayState = 0;
	for (int j = CMD_PREFFIX_LEN; j < CMD_PREFFIX_LEN + RELAY_COUNT; j++)
	{
		_resultBuffer[j] = KMPProDinoESP32.getRelayState(relayState++) ? CH_1 : CH_0;
	}
	
	_resultBuffer[CMD_PREFFIX_LEN + RELAY_COUNT] = CH_NONE;

	Serial.println("Transmitting relays statuses...");
	Serial.println(_resultBuffer);

	// Transmit result.
	KMPProDinoESP32.rs485Write(_resultBuffer);
}

/**
* @brief Loop void. Arduino executed second.
*
*
* @return void
*/
void loop() {
	KMPProDinoESP32.processStatusLed(green, 1000);
	// Waiting for a data.
	int i = KMPProDinoESP32.rs485Read();

	if (i == -1)
	{
		return;
	}

	Serial.println("Receiving data...");

	// If in RS485 port has any data - Status led is Yellow
	KMPProDinoESP32.setStatusLed(yellow);

	uint8_t buffPos = 0;

	// Reading data from the RS485 port.
	while (i > -1 && buffPos < BUFF_MAX)
	{
		// Adding received data in a buffer.
		_dataBuffer[buffPos++] = (char)i;
		Serial.write((char)i);
		// Reading a next char.
		i = KMPProDinoESP32.rs485Read();
	}

	_dataBuffer[buffPos] = CH_NONE;

	Serial.println();

	ProcessData();

	KMPProDinoESP32.offStatusLed();
}

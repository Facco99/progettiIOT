#include "KMPProDinoESP32.h"
#include "KMPCommon.h"
#include "arduino_secrets.h"

#include <WiFi.h>
#include <WiFiClient.h>

// If you stay this define uncommented (this situation), the example supports only boards with Ethernet
// If you commet this define, the example supports only boards with WiFi 
//#define ETH_TEST

#ifdef ETH_TEST
	#include "Ethernet/EthernetClient.h"
	#include <../Blynk/src/Adapters/BlynkEthernet.h>
	static EthernetClient _blynkEthernetClient;
	static BlynkArduinoClient _blynkTransport(_blynkEthernetClient);
	BlynkEthernet Blynk(_blynkTransport);
	#include <BlynkWidgets.h>
#else
	#include <BlynkSimpleEsp32.h>
#endif // ETH_TEST

//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

// Check sensor data, interval in milliseconds.
const long CHECK_HT_INTERVAL_MS = 10000;
// Store last measure time.
unsigned long _mesureTimeout;				

// Opto input structure.
struct OptoIn_t
{
	OptoIn Input;
	WidgetLED Widget;
	bool Status;
};

// Store opto input data, settings and processing objects.
OptoIn_t _optoInputs[OPTOIN_COUNT] =
{
	{ OptoIn1, WidgetLED(V5), false },
	{ OptoIn2, WidgetLED(V6), false },
	{ OptoIn3, WidgetLED(V7), false },
	{ OptoIn4, WidgetLED(V8), false }
};

/**
* @brief Setup void. Ii is Arduino executed first. Initialize DiNo board.
*
* @return void
*/
void setup()
{
	delay(5000);
	Serial.begin(115200);
	Serial.println("The example BlynkWE is starting...");

	// Init Dino board. Set pins, start GSM.
#ifndef ETH_TEST
	KMPProDinoESP32.begin(ProDino_ESP32);
	//KMPProDinoESP32.begin(ProDino_ESP32_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_LoRa_RFM);
#else
	KMPProDinoESP32.begin(ProDino_ESP32_Ethernet);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_GSM);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa);
	//KMPProDinoESP32.begin(ProDino_ESP32_Ethernet_LoRa_RFM);
#endif
	KMPProDinoESP32.setStatusLed(blue);

	_mesureTimeout = 0;

#ifdef ETH_TEST
	Blynk.begin(AUTH_TOKEN);
	// You can also specify server:
	//Blynk.begin(AUTH_TOKEN, "blynk-cloud.com", 80);
	//Blynk.begin(AUTH_TOKEN, IPAddress(192,168,1,100), 8080);
#else
	Blynk.begin(AUTH_TOKEN, SSID_NAME, SSID_PASSWORD);
	// You can also specify server:
	//Blynk.begin(AUTH_TOKEN, SSID_NAME, SSID_PASSWORD, "blynk-cloud.com", 80);
	//Blynk.begin(AUTH_TOKEN, SSID_NAME, SSID_PASSWORD, IPAddress(192,168,1,100), 8080);
#endif

	KMPProDinoESP32.offStatusLed();

	Serial.println("The example BlynkWE is started.");
}


/**
* @brief Processing data from isolated inputs. It will send data to Blynk if only the input statuses were changed.
*
* @return void
*/
void ProcessOptoInputs(bool force)
{
	for (int i = 0; i < OPTOIN_COUNT; i++)
	{
		OptoIn_t* optoInput = &_optoInputs[i];
		bool currentStatus = KMPProDinoESP32.getOptoInState(optoInput->Input);
		if (optoInput->Status != currentStatus || ((bool)optoInput->Widget.getValue()) != currentStatus || force)
		{
			Serial.println("Opto input " + String(i + 1) + " status changed to -> \"" + currentStatus + "\". WidgetLED value: " + optoInput->Widget.getValue());

			currentStatus ? optoInput->Widget.on() : optoInput->Widget.off();
			optoInput->Status = currentStatus;
		}
	}
}

/**
 * @brief Set relay state.
 *
 * @return void
 */
void SetRelay(Relay relay, int status)
{
	KMPProDinoESP32.setRelayState(relay, status == 1);
}

/*****************************
* Blynk methods.
*****************************/
/**
 * @brief This function will be run every time when Blynk connection is established.
 *
 */
BLYNK_CONNECTED() {
	// Request Blynk server to re-send latest values for all pins.
	Blynk.syncAll();

	ProcessOptoInputs(true);
}

/**
 * @brief Set Relay 1 state.
 *			On virtual pin 1.
 */
BLYNK_WRITE(V1)
{
	SetRelay(Relay1, param.asInt());
}

/**
* @brief Set Relay 1 state.
*			On virtual pin 1.
*/
BLYNK_READ(V1)
{
	Blynk.virtualWrite(V1, KMPProDinoESP32.getRelayState(Relay1));
}

/**
 * @brief Set Relay 2 state.
 *			On virtual pin 2.
 */
BLYNK_WRITE(V2)
{
	SetRelay(Relay2, param.asInt());
}

/**
 * @brief Set Relay 3 state.
 *			On virtual pin 3.
 */
BLYNK_WRITE(V3)
{
	SetRelay(Relay3, param.asInt());
}

/**
 * @brief Set Relay 4 state.
 *			On virtual pin 4.
 */
BLYNK_WRITE(V4)
{
	SetRelay(Relay4, param.asInt());
}

/**
* @brief Loop void. Arduino executed second.
*
* @return void
*/
void loop(void)
{
	KMPProDinoESP32.processStatusLed(green, 1000);

	ProcessOptoInputs(false);

	Blynk.run();
}

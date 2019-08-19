/*
 * Arduino Outdoor Weather Station Project
 *
 * REQUIRES the following Arduino libraries:
 *	- SPI						------	SPI Master library for Arduino (Arduino built-in)
 *	- NRFLite					2.2.2	nRF24L01+ 2.4 GHz Transceiver library: https://github.com/dparson55/NRFLite
 *	- Arduino_MKRENV			1.1.0	MKRENV Library for Arduino: https://github.com/arduino-libraries/Arduino_MKRENV
 *	- Wire						------	I2C interface library (Arduino built-in) (required by Arduino_MKRENV)
 *	- WiFiNINA					1.4.0	Arduino MKR 1010 library: https://www.arduino.cc/en/Reference/WiFiNINA
 */


/*
 * Include libraries
 */
#include "Arduino.h"
#include <SPI.h>				// SPI Master library for Arduino (required by NRFLite.h)
#include <NRFLite.h>			// nRF24L01+ Transceiver library
#include <Arduino_MKRENV.h>		// MKRENV Library for Arduino


/*
 * Debug initialize
 */
#define DEBUG // activate "debug switch"
#ifdef DEBUG
#define DBUG_PRINT(x) Serial.print(x)		// activate print
#define DEBUG_PRINTLINE(x) Serial.println(x)	// activate print line
#else
#define DEBUG_PRINT(x)						// do nothing
#define DEBUG_PRINTLINE(x)					// do nothing
#endif


/*
 * MKR ENV initialize
 */
// No definitions required by the MKR ENV shield.


/*
 * nRF24L01+ Radio TX initialize
 */

// Radio    Arduino
// CE    -> 1
// CSN   -> 2	(Hardware SPI SS)
// MOSI  -> 11	(Hardware SPI MOSI)
// MISO  -> 12	(Hardware SPI MISO)
// SCK   -> 13	(Hardware SPI SCK)
// IRQ   -> No	connection
// VCC   -> No	more than 3.6 volts
// GND   -> GND

const static uint8_t RADIO_ID = 1;             // Our radio's id.
const static uint8_t DESTINATION_RADIO_ID = 0; // Id of the radio we will transmit to.
const static uint8_t PIN_RADIO_CE = 1;
const static uint8_t PIN_RADIO_CSN = 2;

struct RadioPacket // Any packet up to 32 bytes can be sent.
{
	float outTemp;
	float outHum;
	float outPress;
	float outIllum;
	float outUva;
	float outUvb;
	float outUvIndex;
};

NRFLite _radio;
RadioPacket _radioData;




/*****************
 * ARDUINO SETUP *
 ****************/
void setup()
{

	/*
	 * Start serial port & serial monitor to Baud rate 9600 bits/s
	 */
	Serial.begin(9600); // serial return of Arduino's output
	//DEBUG_PRINTLINE("my debug message");


	/*
	 * Start MKR ENV Shield
	 */
	if (!ENV.begin()) {
		Serial.println("Failed to initialize MKR ENV shield!");
		while (1);
	}


	/*
	 * Start nRF24L01+ Radio
	 */
	// By default, 'init' configures the radio to use a 2MBPS bitrate on channel 100 (channels 0-125 are valid).
	// Both the RX and TX radios must have the same bitrate and channel to communicate with each other.
	// You can run the 'ChannelScanner' example to help select the best channel for your environment.
	// You can assign a different bitrate and channel as shown below.
	//   _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE250KBPS, 0)
	//   _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE1MBPS, 75)
	_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE2MBPS, 100); // THE DEFAULT

	if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN))
	{
		Serial.println("Cannot communicate with radio");
		while (1); // Wait here forever.
	}

}




/****************
 * ARDUINO LOOP *
 ***************/
void loop()
{

	/*
	 * MKR ENV read
	 */
	// read all the sensor values
	float temperature = ENV.readTemperature();
	float humidity    = ENV.readHumidity();
	float pressure    = ENV.readPressure();
	float illuminance = ENV.readIlluminance();
	float uva         = ENV.readUVA();
	float uvb         = ENV.readUVB();
	float uvIndex     = ENV.readUVIndex();

	// print each of the sensor values
	Serial.print("Temperature = ");
	Serial.print(temperature);
	Serial.println(" °C");

	Serial.print("Humidity    = ");
	Serial.print(humidity);
	Serial.println(" %");

	Serial.print("Pressure    = ");
	Serial.print(pressure);
	Serial.println(" kPa");

	Serial.print("Illuminance = ");
	Serial.print(illuminance);
	Serial.println(" lx");

	Serial.print("UVA         = ");
	Serial.println(uva);

	Serial.print("UVB         = ");
	Serial.println(uvb);

	Serial.print("UV Index    = ");
	Serial.println(uvIndex);

	// print an empty line
	Serial.println();


	/*
	 * nRF24L01+ Radio TX
	 */
	_radioData.outTemp = temperature;
	_radioData.outHum = humidity;
	_radioData.outPress = pressure;
	_radioData.outIllum = illuminance;
	_radioData.outUva = uva;
	_radioData.outUvb = uvb;
	_radioData.outUvIndex = uvIndex;

	Serial.println("Sending:");
	Serial.println(_radioData.outTemp);
	Serial.println(_radioData.outHum);
	Serial.println(_radioData.outPress);
	Serial.println(_radioData.outIllum);
	Serial.println(_radioData.outUva);
	Serial.println(_radioData.outUvb);
	Serial.println(_radioData.outUvIndex);

	// By default, 'send' transmits data and waits for an acknowledgement.  If no acknowledgement is received,
	// it will try again up to 16 times.  You can also perform a NO_ACK send that does not request an acknowledgement.
	// The data packet will only be transmitted a single time so there is no guarantee it will be successful.  Any random
	// electromagnetic interference can sporatically cause packets to be lost, so NO_ACK sends are only suited for certain
	// types of situations, such as streaming real-time data where performance is more important than reliability.
	//   _radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData), NRFLite::NO_ACK)
	_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData), NRFLite::REQUIRE_ACK); // THE DEFAULT

	if (_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData))) // Note how '&' must be placed in front of the variable name.
	{
		Serial.println("...Success");
	}
	else
	{
		Serial.println("...Failed");
	}

	delay(1000);


}




/*************
 * FUNCTIONS *
 ************/

/*
 * Debug functions
 */
void print (const char str[], int number) {
	char buf[100];
	sprintf(buf, "%s %d", str, number);
	DEBUG_PRINTLINE(buf);
}

void print (const char str[], float number) {
	char buf[100];
	char strbuf[10];
	dtostrf(number, 3, 3, strbuf);
	sprintf(buf, "%s %s", str, strbuf);
	DEBUG_PRINTLINE(buf);
}

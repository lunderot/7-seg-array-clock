#include <LEDMatrixDriver.hpp>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "wifi_config.h"

const int NO_OF_DRIVERS = 1;
LEDMatrixDriver lmd(NO_OF_DRIVERS, SS);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, 3600 * 2);

void setup()
{
	lmd.setEnabled(true);
	lmd.setIntensity(2);
	lmd.setScanLimit(7);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
	}

	timeClient.begin();
}

int counter = 0;

uint8_t segs[] = {
	0b01111110,
	0b00110000,
	0b01101101,
	0b01111001,
	0b00110011,
	0b01011011,
	0b01011111,
	0b01110000,
	0b01111111,
	0b01111011,
};

void loop()
{
	timeClient.update();
	uint8_t *buffer = lmd.getFrameBuffer();
	int hour = timeClient.getHours();
	int min = timeClient.getMinutes();
	buffer[0] = segs[(hour / 10) % 10];
	buffer[1] = segs[hour % 10];
	buffer[2] = segs[(min / 10) % 10];
	buffer[3] = segs[min % 10];

	lmd.display();
	delay(5000);
}

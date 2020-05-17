#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <NTPClient.h>
#include "wifi_config.h"
#include "bigsegs.h"

#define NUM_CHIPS 18
#define NUM_DIGITS (NUM_CHIPS * 8)
#define DISPLAY_WIDTH 24
#define DISPLAY_HEIGHT 6

SPISettings spiSettings(5000000, MSBFIRST, SPI_MODE0);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600 * 2, 60000);

uint8_t data[NUM_DIGITS] = {0};
int count = 0;

void sendAll(uint16_t command)
{
	SPI.beginTransaction(spiSettings);
	digitalWrite(SS, 0);
	for (size_t i = 0; i < NUM_CHIPS; i++)
	{
		SPI.transfer16(command);
	}
	digitalWrite(SS, 1);
	SPI.endTransaction();
}

void sendBuffer(uint8_t *buffer)
{
	for (size_t j = 0; j < 8; j++)
	{
		SPI.beginTransaction(spiSettings);
		digitalWrite(SS, 0);
		for (size_t i = 0; i < NUM_CHIPS; i++)
		{
			int index = ((NUM_DIGITS - 8 * i) - 8) + j;
			int dig = (j + 1) << 8;
			SPI.transfer16(dig | buffer[index]);
		}
		digitalWrite(SS, 1);
		SPI.endTransaction();
	}
}

void writeBigSeg(uint8_t *data, uint8_t *const segs, uint8_t digit, uint8_t offset, bool orFirst)
{
	for (size_t i = 0; i < 6; i++)
	{
		uint8_t *dst = data + i * DISPLAY_WIDTH + offset;
		uint8_t *src = bigsegs + 36 * digit + i * 6;
		if (orFirst)
		{
			*dst |= *src;
			memcpy(dst + 1, src + 1, 5);
		}
		else
		{
			memcpy(dst, src, 6);
		}
	}
}

void writeDots(uint8_t *data, bool state, uint8_t on, uint8_t off)
{
	data[(24 * 1 + 12)] = state ? on : off;
	data[(24 * 4 + 11)] = state ? on : off;
}

void setup()
{
	pinMode(SS, OUTPUT);
	digitalWrite(SS, 1);
	SPI.begin();
	SPI.setHwCs(false);

	sendAll(0x0F00); //Disable display test
	sendAll(0x0900); //Decode mode none
	sendAll(0x0B07); //Scan limit all
	sendAll(0x0A02); //Intensity 2
	sendAll(0x0C01); //Enable display

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(200);
		data[0] ^= 0b10000000;
		sendBuffer(data);
	}
	data[0] = 0b00000000;
	timeClient.begin();
}

void loop()
{
	timeClient.update();
	int hour = timeClient.getHours();
	int minute = timeClient.getMinutes();
	int seconds = timeClient.getSeconds();

	writeBigSeg(data, bigsegs, hour / 10, 0, false);
	writeBigSeg(data, bigsegs, hour % 10, 5, true);
	writeBigSeg(data, bigsegs, minute / 10, 13, false);
	writeBigSeg(data, bigsegs, minute % 10, 18, true);
	writeDots(data, seconds % 2, 0b00011101, 0b00000000);
	if (++count % 30 == 0)
	{
		count = 0;
		sendAll(0x0A02);
		sendAll(0x0F00);
		sendAll(0x0C01);
	}
	sendBuffer(data);
	delay(1000);
}

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

void clock_function() {
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
}

uint8_t pattern[] = {0x80,
0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x63,0x63,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x63,0x00,0x60,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,
0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x63,0x63,0x63,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0xF3,0xFF,0x60,0x63,0x77,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x9D,
0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x63,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x73,0xFF,0xFF,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0x9C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x63,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x63,0xFF,0xFF,0xFF,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x62,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x9D,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9D,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x63,0x63,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0xFF,0xFF,0xFF,0xFF,0xF7,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x62,0x63,0xFF,
0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9D,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x63,0x63,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x63,0x77,0xFF,
0xFF,0xFF,0xFF,0xBF,0x9D,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x63,0x63,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0xF7,0xFF,0xFF,
0xFF,0xFF,0xBF,0x9D,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x63,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0x77,0xFF,0xFF,0xFF,
0xFF,0xFF,0x9F,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x63,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x63,0x73,0xFF,0xFF,0xFF,0xFF,
0xFF,0x9F,0x9C,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x63,0x63,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x62,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,
0xBF,0x9D,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9D,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x63,0x63,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0xBF,
0x9D,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x63,0x63,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x63,0x63,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0x7F,0x9D,
0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0xFF,0xFF,0xFF,0x9F,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x63,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x62,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x88,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0xFF,0xFF,0xBF,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x63,0x63,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0xFF,0xFF,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x63,0x63,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x42,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0x08,0xBF,0x9D,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x9C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x63,0x63,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x62,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x0C,0x00,0x9D,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x63,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x62,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x62,0x40,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x8C,0x00,0x00,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0xFF,0xFF,0xBF,0x9D,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x63,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x42,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x8C,0xFF,0xBF,0x9D,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x63,0x63,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x62,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x42,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9C,0x00,0xFF,0x9F,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x63,0x63,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x42,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x9C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9D,0x00,0x00,0x9F,0x9D,0x9D,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x63,0x63,0x63,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x62,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9D,0x0C,0x00,0x00,0x9D,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x63,0x63,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x00,0x00,0x00,0x00,0x00,0x60,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x62,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x98,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x0C,0x00,0x00,0x00,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x63,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x00,0x00,0x00,0x00,0x60,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x98,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x8C,0x00,0x00,0x00,0x00,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9D,0x63,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x63,0x00,0x00,0x00,0x00,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x62,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x80,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9C,0x00,0x00,0x00,0x00,0x00,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9D,0x9D,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x63,0x63,0x00,0x00,0x00,0x62,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x80,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9D,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0xBD,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x63,0x63,0x63,0x00,0x00,0x60,0xF3,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x98,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9D,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9D,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x67,0x63,0x63,0x63,0x73,0x00,0x60,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x98,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9D,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x63,0x63,0x63,0x73,0xFF,0x40,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x63,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x88,0x9D,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x9D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0x9D,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x63,0x63,0x73,0xFF,0xFF,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x67,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x62,0x73,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x80,0x9C,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9D,0x9D,0x9D,0xBD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x77,0x63,0x63,0x63,0x63,0xFF,0xFF,0xFF,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x00,0x00,0x62,0x63,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x63,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
#define NUM_FRAMES 30

void pattern_function() {
	if (++count % NUM_FRAMES == 0)
	{
		count = 0;
		sendAll(0x0A02);
		sendAll(0x0F00);
		sendAll(0x0C01);
	}
	sendBuffer(pattern+(144*count));
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
	pattern_function();
	delay(33);
}

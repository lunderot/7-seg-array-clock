#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include "wifi_config.h"

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

#define NUM_CHIPS 18
#define NUM_DIGITS NUM_CHIPS * 8

SPISettings spiSettings(5000000, MSBFIRST, SPI_MODE0);

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
			int index = (((NUM_CHIPS * 8) - 8 * i) - 8) + j;
			int dig = (j + 1) << 8;
			SPI.transfer16(dig | buffer[index]);
		}
		digitalWrite(SS, 1);
		SPI.endTransaction();
	}
}

uint8_t data[144] = {0};

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
	sendAll(0x0100); //Set all digits to off
	sendAll(0x0200);
	sendAll(0x0300);
	sendAll(0x0400);
	sendAll(0x0500);
	sendAll(0x0600);
	sendAll(0x0700);
	sendAll(0x0800);
	sendAll(0x0C01); //Enable display

	for (size_t i = 0; i < 144; i++)
	{
		data[i] = segs[random(10)];
	}
}

void loop()
{
	data[random(144)] = segs[random(10)];
	sendBuffer(data);
	delay(100);
}

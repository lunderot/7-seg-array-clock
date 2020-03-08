#include <LEDMatrixDriver.hpp>

const int NO_OF_DRIVERS = 1;
LEDMatrixDriver lmd(NO_OF_DRIVERS, SS);

void setup()
{
	lmd.setEnabled(true);
	lmd.setIntensity(2);
	lmd.setScanLimit(7);
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

	uint8_t *buffer = lmd.getFrameBuffer();
	buffer[0] = segs[(counter / 1000) % 10];
	buffer[1] = segs[(counter / 100) % 10];
	buffer[2] = segs[(counter / 10) % 10];
	buffer[3] = segs[counter % 10];

	lmd.display();
	counter++;
	delay(1);
}

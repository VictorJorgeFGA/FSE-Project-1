#include "DHT22.hpp"
#include <cstdint>

DHT22::DHT22(int bcm_gpio_port, int read_time_freq):
port(bcm_gpio_port), time_freq(read_time_freq)
{

}

DHT22::~DHT22()
{

}

bool DHT22::should_read() const
{
    struct timeval current_time;
    gettimeofday(&current_time, nullptr);

    return (current_time.tv_usec - last_read_time.tv_usec) / 1000 >= time_freq;
}

void DHT22::read()
{
#ifndef _DEVELOPMENT_MODE_
    int data[5] = { 0, 0, 0, 0, 0 };
    uint8_t laststate = HIGH;
	uint8_t counter	= 0;
	uint8_t j = 0;
	uint8_t i;

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	/* pull pin down for 18 milliseconds */
	pinMode(port, OUTPUT);
	digitalWrite(port, LOW);
	delay(18);

	/* prepare to read the pin */
	pinMode(port, INPUT);

	/* detect change and read data */
	for ( i = 0; i < 85; i++ ) {
		counter = 0;
		while ( digitalRead( port ) == laststate ) {
			counter++;
			delayMicroseconds( 1 );
			if ( counter == 255 ) {
				break;
			}
		}
		laststate = digitalRead( port );

		if ( counter == 255 )
			break;

		/* ignore first 3 transitions */
		if ( (i >= 4) && (i % 2 == 0) ) {
			/* shove each bit into the storage bytes */
			data[j / 8] <<= 1;
			if ( counter > 16 )
				data[j / 8] |= 1;
			j++;
		}
	}

	/*
	 * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
	 * print it out if data is good
	 */
	if ( (j >= 40) && (data[4] == ( (data[0] + data[1] + data[2] + data[3]) & 0xFF) ) ) {
		float h = (float)((data[0] << 8) + data[1]) / 10;
		if ( h > 100 ) {
			h = data[0];	// for DHT11
		}
		float c = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
		if ( c > 125 ) {
			c = data[2];	// for DHT11
		}
		if ( data[2] & 0x80 ) {
			c = -c;
		}
		temp_cels = c;
		humidity = h;
	} else {
		temp_cels = humidity = -1;
	}
    gettimeofday(&last_read_time, nullptr);
#endif
}

float DHT22::get_temperature() const
{
    return temp_cels;
}

float DHT22::get_humidity() const
{
    return humidity;
}

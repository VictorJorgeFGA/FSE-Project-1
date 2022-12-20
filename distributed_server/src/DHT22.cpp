#include "DHT22.hpp"
#include <cstdint>
#include <sys/time.h>
#include <time.h>
// #include <wiringPi.h>

#define MAX_TIMINGS 85

static int durn(struct timespec t1, struct timespec t2) {
	return(((t2.tv_sec-t1.tv_sec)*1000000) + ((t2.tv_nsec-t1.tv_nsec)/1000));	// elapsed microsecs
}

DHT22::DHT22(int t_pin, int t_cycles_to_read):
m_pin(t_pin), m_cycles_to_read(t_cycles_to_read), m_temp_cels(0), m_previous_temp_cels(0), m_humidity(0), m_previous_humidity(0)
{

}

DHT22::~DHT22()
{

}

void DHT22::refresh_state(int t_elapsed_cycles)
{
	m_previous_temp_cels = m_temp_cels;
	m_previous_humidity = m_humidity;
	if (t_elapsed_cycles % m_cycles_to_read != 0)
		return;

	get_sensor_data();
}

bool DHT22::previous_and_current_state_differ() const noexcept
{
	return (m_temp_cels != m_previous_temp_cels || m_humidity != m_previous_humidity);
}

void DHT22::sync_previous_and_current_states() noexcept
{
    m_previous_temp_cels = m_temp_cels;
    m_previous_humidity = m_humidity;
}

int DHT22::temperature() const noexcept
{
	return m_temp_cels;
}

int DHT22::humidity() const noexcept
{
	return m_humidity;
}

void DHT22::get_sensor_data()
{
    #ifndef _DEVELOPMENT_MODE_
	int data[5] = {0,0,0,0,0};
	float Temp  = 0.0;
    float Hum   = 0.0;
    bool  Valid = false;
	bool Fh = false;

	// Signal Sensor we're ready to read by pulling pin UP for 10 mS.
    // pulling pin down for 18 mS and then back up for 40 µS.
    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, HIGH);
    delay(10);
    digitalWrite(m_pin, LOW);
    delay(18);
    digitalWrite(m_pin, HIGH);
    delayMicroseconds(40);
    pinMode(m_pin, INPUT);

	struct timespec	st, cur;
    int uSec = 0;
    int Toggles   = 0;
    int BitCnt    = 0;
    int lastState = HIGH;

	// Read data from sensor.
    for(Toggles=0; (Toggles < MAX_TIMINGS) && (uSec < 255); Toggles++) {

        clock_gettime(CLOCK_REALTIME, &st);
        while((digitalRead(m_pin)==lastState) && (uSec < 255) ) {
            clock_gettime(CLOCK_REALTIME, &cur);
            uSec=durn(st,cur);
        };

        lastState = digitalRead(m_pin);

        // First 2 state changes are sensor signaling ready to send, ignore them.
        // Each bit is preceeded by a state change to mark its beginning, ignore it too.
        if( (Toggles > 2) && (Toggles % 2 == 0)){
            // Each array element has 8 bits.  Shift Left 1 bit.
            data[ BitCnt / 8 ] <<= 1;
            // A State Change > 35 µS is a '1'.
            if(uSec>35) data[ BitCnt/8 ] |= 0x00000001;

            BitCnt++;
        }
    }

	// Read 40 bits. (Five elements of 8 bits each)  Last element is a checksum.
    if((BitCnt >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
        Valid= true;
        Hum  = (float)((data[0] << 8) + data[1]) / 10.0;
        Temp = (float)((data[2] << 8) + data[3]) / 10.0;
        if(data[2] & 0x80)  Temp *= -1;         // Negative Sign Bit on.
        if(Fh){ Temp *= 1.8; Temp += 32.0; }    // Convert to Fahrenheit
    }
    else {                                      // Data Bad, use cached values.
        Valid= false;
        Hum  = 0.0;
        Temp = 0.0;
    }

	if (Valid) {
		m_temp_cels = Temp * 10;
		m_humidity = Hum * 10;
	}
    #endif
}

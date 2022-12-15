#ifndef _DHTT22_HPP_
#define _DHTT22_HPP_

#include "Environment.hpp"
#include "Device.hpp"

#ifndef _DEVELOPMENT_MODE_
    #include <wiringPi.h>
#endif

#include <sys/time.h>

class DHT22
{
public:
    // bcm_gipio_port: Port where the sensor is located; read_time_freq: gap between reads (in milliseconds)
    DHT22(int bcm_gpio_port, int read_time_freq);
    ~DHT22();
    bool should_read() const;
    void read();

    float get_temperature() const;
    float get_humidity() const;

private:
    int port;
    int time_freq;
    float temp_cels;
    float humidity;
    struct timeval last_read_time;
    DHT22();
    DHT22(DHT22&);
};

#endif
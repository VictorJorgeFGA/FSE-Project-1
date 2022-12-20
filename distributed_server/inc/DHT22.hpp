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
    DHT22(int t_pin, int t_cycles_to_read = 40);
    ~DHT22();

    void refresh_state(int t_elapsed_cycles);

    bool previous_and_current_state_differ() const noexcept;
    void sync_previous_and_current_states() noexcept;

    int temperature() const noexcept;
    int humidity() const noexcept;

private:
    void get_sensor_data();

    DHT22();
    DHT22(DHT22&);
    int m_pin;
    int m_cycles_to_read;
    int m_temp_cels;
    int m_previous_temp_cels;
    int m_humidity;
    int m_previous_humidity;
};

#endif
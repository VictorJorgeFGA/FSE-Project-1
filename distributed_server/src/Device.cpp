#include "Device.hpp"
#include "Environment.hpp"
#include <iostream>

#ifndef _DEVELOPMENT_MODE_
#include <wiringPi.h>
#endif

Device::Device(int t_pin, int t_mode, int t_refresh_time_delta):
m_pin(t_pin), m_mode(t_mode), m_last_state(0), m_refresh_time_delta(t_refresh_time_delta)
{
    #ifndef _DEVELOPMENT_MODE_
    pinMode(m_pin, m_mode);
    if (m_mode == OUTPUT)
        digitalWrite(m_pin, LOW);
    #endif
}

Device::~Device()
{

}

void Device::set_state(int t_new_state)
{
    #ifndef _DEVELOPMENT_MODE_
    if (m_mode != OUTPUT) {
        std::cerr << "\033[103m\033[1;31mERROR\033[0m: Attempt to write on input device pin: " << m_pin << std::endl;
        exit(EXIT_FAILURE);
    }
    if (t_new_state != HIGH && t_new_state != LOW) {
        std::cerr << "\033[103m\033[1;31mERROR\033[0m: Attempt to set an invalid state (" << t_new_state << ") on device pin=" << m_pin << std::endl;
        exit(EXIT_FAILURE);
    }
    digitalWrite(m_pin, t_new_state);
    m_last_state = t_new_state;
    #else
    m_last_state = t_new_state;
    #endif
}

int Device::get_last_state() noexcept
{
    #ifdef _DEVELOPMENT_MODE_
    m_last_state = rand() % 2;
    #endif
    return m_last_state;
}

int Device::refresh_time_delta() const noexcept
{
    return m_refresh_time_delta;
}

void Device::toggle_state()
{
    #ifndef _DEVELOPMENT_MODE_
    if (get_last_state() == HIGH)
        set_state(LOW);
    else
        set_state(HIGH);
    #endif
}

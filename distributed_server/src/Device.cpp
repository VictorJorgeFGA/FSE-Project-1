#include "Device.hpp"
#include "Environment.hpp"
#include <iostream>

#ifndef _DEVELOPMENT_MODE_
#include <wiringPi.h>
#endif

#include "Helpers.hpp"

Device::Device(int t_bcm_pin, int t_mode)
{
    m_pin = Helpers::gpio_bcm_to_wiringPi_pin(t_bcm_pin);
    m_mode = t_mode;
    m_last_state = 0;
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
        std::cerr << "\033[0;33mWARNING:\033[0m Attempt to write on input device pin: " << m_pin << std::endl;
        exit(EXIT_FAILURE);
    }
    if (t_new_state != HIGH && t_new_state != LOW) {
        std::cerr << "Attempt to set an invalid state (" << t_new_state << ") on device pin=" << m_pin << std::endl;
        exit(EXIT_FAILURE);
    }
    digitalWrite(m_pin, t_new_state);
    m_last_state = t_new_state;
    #else
    m_last_state = t_new_state;
    #endif
}

int Device::get_state()
{
    #ifndef _DEVELOPMENT_MODE_
    if (m_mode == INPUT)
        m_last_state = digitalRead(m_pin);
    #else
    m_last_state = rand() % 2;
    #endif
    return m_last_state;
}

void Device::toggle_state()
{
    int last_state = get_state();
    #ifndef _DEVELOPMENT_MODE_
    if (last_state == HIGH) {
        set_state(LOW);
    }
    else {
        set_state(HIGH);
    }
    #endif
}

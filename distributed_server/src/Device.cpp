#include "Device.hpp"
#include "Environment.hpp"
#include <iostream>

#ifndef _DEVELOPMENT_MODE_
#include <wiringPi.h>
#endif

Device::Device(int t_pin, int t_mode, int t_cycles_to_read):
m_pin(t_pin), m_mode(t_mode), m_current_state(0), m_previous_state(0), m_cycles_to_read(t_cycles_to_read)
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
    m_previous_state = m_current_state;
    m_current_state = t_new_state;
    #else
    m_previous_state = m_current_state;
    m_current_state = t_new_state;
    #endif
}

void Device::refresh_state(int t_elapsed_cycles)
{
    #ifndef _DEVELOPMENT_MODE_
    if (m_mode != INPUT)
        return;
    if (t_elapsed_cycles % cycles_to_read() == 0) {
        m_current_state = digitalRead(m_pin);
        m_previous_state = m_current_state;
    }
    #endif
}

int Device::get_current_state() noexcept
{
    return m_current_state;
}

bool Device::previous_and_current_state_differ() const noexcept
{
    return m_current_state != m_previous_state;
}

void Device::sync_previous_and_current_states() noexcept
{
    m_previous_state = m_current_state;
}

int Device::cycles_to_read() const noexcept
{
    return m_cycles_to_read;
}

void Device::toggle_state()
{
    #ifndef _DEVELOPMENT_MODE_
    if (get_current_state() == HIGH)
        set_state(LOW);
    else
        set_state(HIGH);
    #else
    if (get_current_state() == 1)
        set_state(0);
    else
        set_state(1);
    #endif
}

#ifndef _DEVICE_HPP_
#define _DEVICE_HPP_

class Device
{
public:
    Device(int t_bcm_pin, int t_mode);
    ~Device();

    void set_state(int t_new_state);
    int get_state();
    void toggle_state();

private:
    Device();
    int m_pin;
    int m_mode;
    int m_last_state;
};

#endif

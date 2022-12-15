#ifndef _DEVICE_HPP_
#define _DEVICE_HPP_

class Device
{
public:
    Device(int t_pin, int t_mode, int t_refresh_time_delta = -1);
    ~Device();

    void set_state(int t_new_state);
    int get_last_state() noexcept;

    int refresh_time_delta() const noexcept;

    void toggle_state();

private:
    Device();
    int m_pin;
    int m_mode;
    int m_last_state;
    int m_refresh_time_delta; // in milliseconds
};

#endif

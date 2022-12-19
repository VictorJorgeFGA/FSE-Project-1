#ifndef _DEVICE_HPP_
#define _DEVICE_HPP_

class Device
{
public:
    Device(int t_pin, int t_mode, int t_cycles_to_read = 1);
    ~Device();

    void set_state(int t_new_state);
    void toggle_state();

    void refresh_state(int t_elapsed_cycles);
    bool state_changed_this_cycle();
    int get_current_state() noexcept;

    int cycles_to_read() const noexcept;

private:
    Device();
    int m_pin;
    int m_mode;
    int m_current_state;
    int m_previous_state;
    int m_cycles_to_read; // in milliseconds
};

#endif

#ifndef _CONFIGURATION_HANDLER_HPP_
#define _CONFIGURATION_HANDLER_HPP_

#include "JSON.hpp"
#include <string>
#include <map>

class ConfigurationHandler
{
public:
    ConfigurationHandler();
    ~ConfigurationHandler();
    std::string main_server_address() const;
    int main_server_port() const;
    int wiringPi_pin_for(std::string & device);
    int wiringPi_pin_for(std::string && device);

private:
    void initialize_sensors();
    void initialize_temp_hum_sensor();
    JSON * read_configuration_from_file(const std::string &);
    JSON * read_configuration_from_file(const char *);
    int bcm_to_wiringPi(int bcm_pin) const noexcept;

    std::string config_schema;
    std::string m_main_server_address;
    int m_main_server_port;
    std::map<std::string, int> m_devices_pins;
};

#endif

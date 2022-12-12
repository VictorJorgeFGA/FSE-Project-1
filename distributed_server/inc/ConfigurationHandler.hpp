#ifndef _CONFIGURATION_HANDLER_HPP_
#define _CONFIGURATION_HANDLER_HPP_

#include "JSON.hpp"

class ConfigurationHandler
{
public:
    ConfigurationHandler(int argc, char * argv[]);
    ~ConfigurationHandler();
    void print_usage_message() const;
    void print_running_mode_message() const;
    JSON * get_config();
private:
    void initialize_sensors();
    void initialize_temp_hum_sensor();
    JSON * read_configuration_from_file(const std::string &);
    JSON * config_json;
    std::string mode;
    std::string config_schema;
};

#endif

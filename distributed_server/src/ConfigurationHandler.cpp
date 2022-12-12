#include "ConfigurationHandler.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include "Environment.hpp"

#ifndef _DEVELOPMENT_MODE_
    #include <wiringPi.h>
#endif

ConfigurationHandler::ConfigurationHandler(int argc, char * argv[])
{
    if (argc < 2 || argc > 3) {
        print_usage_message();
        exit(1);
    }
    mode = std::string(argv[1]);
    if (mode == "main" && argc == 2) {
        config_schema = "0";
    }
    else if (mode == "distributed" && argc == 3) {
        config_schema = std::string(argv[2]);
        if (config_schema != "1" && config_schema != "2") {
            print_usage_message();
            exit(1);
        }
        config_json = read_configuration_from_file("config" + config_schema + ".json");
        initialize_sensors();
    }
    else {
        print_usage_message();
        exit(1);
    }
    print_running_mode_message();
}

ConfigurationHandler::~ConfigurationHandler()
{
    delete config_json;
}

void ConfigurationHandler::print_usage_message() const
{
    std::cout << "Usage: bin/prog <main|distributed> [configuration schema]" << std::endl;
    std::cout << "Where 'main' stands for main server, and 'distributed' for distributed server." << std::endl;
    std::cout << "Configuration schema stands for the port configuration number, i.e. configuration 1 or 2." << std::endl;
}

void ConfigurationHandler::print_running_mode_message() const
{
    std::cout << "Running as a " << mode << " server!" << std::endl;
    if (mode == "distributed")
        std::cout << "Loaded configuration schema: " << config_schema << std::endl << config_json->to_string() << std::endl;
}

JSON * ConfigurationHandler::get_config()
{
    return config_json;
}

void ConfigurationHandler::initialize_sensors()
{
#ifndef _DEVELOPMENT_MODE_
    if (wiringPiSetup() == -1) {
        std::cerr << "An error occurred while trying to setup wiring pi" << std::endl;
        exit(1);
    }

    // Initializing DHT22
    // pinMode((int) cJSON_GetNumberValue(cJSON_GetObjectItem(config_json, "DHT22")), OUTPUT);
#endif
}

void ConfigurationHandler::initialize_temp_hum_sensor()
{

}

JSON * ConfigurationHandler::read_configuration_from_file(const std::string & file_name)
{
    auto file = std::ifstream(file_name.c_str(), std::ifstream::in);
    std::stringstream content;
    content << file.rdbuf();
    return new JSON(content.str());
}

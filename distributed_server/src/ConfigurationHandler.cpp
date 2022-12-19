#include "ConfigurationHandler.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include "Environment.hpp"

#ifndef _DEVELOPMENT_MODE_
    #include <wiringPi.h>
#endif

ConfigurationHandler::ConfigurationHandler()
{
    JSON * config_json = read_configuration_from_file("config.json");
    m_devices_pins["L_01"] = bcm_to_wiringPi(config_json->get_number_from_key("L_01"));
    m_devices_pins["L_02"] = bcm_to_wiringPi(config_json->get_number_from_key("L_02"));
    m_devices_pins["AC"] = bcm_to_wiringPi(config_json->get_number_from_key("AC"));
    m_devices_pins["PR"] = bcm_to_wiringPi(config_json->get_number_from_key("PR"));
    m_devices_pins["AL_BZ"] = bcm_to_wiringPi(config_json->get_number_from_key("AL_BZ"));
    m_devices_pins["SPres"] = bcm_to_wiringPi(config_json->get_number_from_key("SPres"));
    m_devices_pins["SFum"] = bcm_to_wiringPi(config_json->get_number_from_key("SFum"));
    m_devices_pins["SJan"] = bcm_to_wiringPi(config_json->get_number_from_key("SJan"));
    m_devices_pins["SPor"] = bcm_to_wiringPi(config_json->get_number_from_key("SPor"));
    m_devices_pins["SC_IN"] = bcm_to_wiringPi(config_json->get_number_from_key("SC_IN"));
    m_devices_pins["SC_OUT"] = bcm_to_wiringPi(config_json->get_number_from_key("SC_OUT"));
    m_devices_pins["DHT22"] = bcm_to_wiringPi(config_json->get_number_from_key("DHT22"));

    m_main_server_port = config_json->get_number_from_key("main_server_port");
    m_main_server_address = config_json->get_string_from_key("main_server_address");

    #ifndef _DEVELOPMENT_MODE_
    initialize_sensors();
    #endif

    delete config_json;
}

ConfigurationHandler::~ConfigurationHandler()
{
}

std::string ConfigurationHandler::main_server_address() const
{
    return m_main_server_address;
}

int ConfigurationHandler::main_server_port() const
{
    return m_main_server_port;
}

int ConfigurationHandler::wiringPi_pin_for(std::string & device)
{
    if (m_devices_pins.count(device) < 1) {
        std::cerr << "\033[0;31mERROR\033[0m: No registered pin for device " << device << std::endl;
        exit(EXIT_FAILURE);
    }
    return m_devices_pins[device];
}

int ConfigurationHandler::wiringPi_pin_for(std::string && device)
{
    return m_devices_pins[device];
}

void ConfigurationHandler::initialize_sensors()
{
#ifndef _DEVELOPMENT_MODE_
    if (wiringPiSetup() == -1) {
        std::cerr << "\033[0;31mERROR\033[0m: An error occurred while trying to setup wiring pi" << std::endl;
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
    return read_configuration_from_file(file_name.c_str());
}

JSON * ConfigurationHandler::read_configuration_from_file(const char * file_name)
{
    auto file = std::ifstream(file_name, std::ifstream::in);
    std::stringstream content;
    content << file.rdbuf();
    return new JSON(content.str());
}

int ConfigurationHandler::bcm_to_wiringPi(int bcm_pin) const noexcept
{
    switch (bcm_pin)
    {
    case 2:
        return 8;
        break;
    case 3:
        return 9;
        break;
    case 4:
        return 7;
        break;
    case 17:
        return 0;
        break;
    case 27:
        return 2;
        break;
    case 22:
        return 3;
        break;
    case 10:
        return 12;
        break;
    case 9:
        return 13;
        break;
    case 11:
        return 14;
        break;
    case 0:
        return 30;
        break;
    case 5:
        return 21;
        break;
    case 6:
        return 22;
        break;
    case 13:
        return 23;
        break;
    case 19:
        return 24;
        break;
    case 26:
        return 25;
        break;
    case 14:
        return 15;
        break;
    case 15:
        return 16;
        break;
    case 18:
        return 1;
        break;
    case 23:
        return 4;
        break;
    case 24:
        return 5;
        break;
    case 25:
        return 6;
        break;
    case 8:
        return 10;
        break;
    case 7:
        return 11;
        break;
    case 1:
        return 31;
        break;
    case 12:
        return 26;
        break;
    case 16:
        return 27;
        break;
    case 20:
        return 28;
        break;
    case 21:
        return 29;
        break;
    default:
        break;
    }
    std::cerr << "\033[0;31mERROR\033[0m: No wiringPi pin exists for bcm pin " << bcm_pin << std::endl;
    exit(EXIT_FAILURE);
}

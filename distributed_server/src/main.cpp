#include <iostream>
#include "ConfigurationHandler.hpp"
#include "Device.hpp"
#include "Environment.hpp"
#include "DistributedServer.hpp"

#ifndef _DEVELOPMENT_MODE_
    #include <wiringPi.h>
#endif

#ifdef _DEVELOPMENT_MODE_
    #define OUTPUT 1
    #define INPUT 0
#endif

int main(int argc, char * argv[]) {
    auto config = ConfigurationHandler(argc, argv);

    Device lamp_01(config.get_config()->get_number_from_key("L_01"), OUTPUT);
    Device lamp_02(config.get_config()->get_number_from_key("L_02"), OUTPUT);
    Device ac(config.get_config()->get_number_from_key("AC"), OUTPUT);
    Device pr(config.get_config()->get_number_from_key("PR"), OUTPUT);

    std::string ip_address = "127.0.0.1";
    DistributedServer::start_up(ip_address, 3030);

    DistributedServer * server = DistributedServer::server();
    server->add_toggleable_device("lamp_1", &lamp_01);
    server->add_toggleable_device("lamp_2", &lamp_02);
    server->add_toggleable_device("air_conditioner", &ac);
    server->add_toggleable_device("projector", &pr);
    server->run();

    DistributedServer::shut_down();

    return 0;
}
#include <iostream>
#include "ConfigurationHandler.hpp"
#include "Device.hpp"
#include "Environment.hpp"
#include "DistributedServer.hpp"
#include "DHT22.hpp"

#ifndef _DEVELOPMENT_MODE_
    #include <wiringPi.h>
#endif

#ifdef _DEVELOPMENT_MODE_
    #define OUTPUT 1
    #define INPUT 0
#endif

int main(int argc, char * argv[]) {
    auto config = ConfigurationHandler();

    Device lamp_01(config.wiringPi_pin_for("L_01"), OUTPUT);
    Device lamp_02(config.wiringPi_pin_for("L_02"), OUTPUT);
    Device ac(config.wiringPi_pin_for("AC"), OUTPUT);
    Device pr(config.wiringPi_pin_for("PR"), OUTPUT);
    Device buzzer(config.wiringPi_pin_for("AL_BZ"), OUTPUT);

    Device s_pres(config.wiringPi_pin_for("SPres"), INPUT, 10);
    Device s_fum(config.wiringPi_pin_for("SFum"), INPUT, 10);
    Device s_jan(config.wiringPi_pin_for("SJan"), INPUT, 10);
    Device s_por(config.wiringPi_pin_for("SPor"), INPUT, 10);
    Device sc_in(config.wiringPi_pin_for("SC_IN"), INPUT, 1);
    Device sc_out(config.wiringPi_pin_for("SC_OUT"), INPUT, 1);

    DHT22 dht22(config.wiringPi_pin_for("DHT22"));

    DistributedServer::start_up(config.main_server_address(), config.main_server_port());
    DistributedServer * server = DistributedServer::server();

    server->add_w_device("lamp01", &lamp_01);
    server->add_w_device("lamp02", &lamp_02);
    server->add_w_device("air_conditioner", &ac);
    server->add_w_device("projector", &pr);
    server->add_w_device("buzzer_alarm", &buzzer);

    server->add_r_device("presence_sensor", &s_pres);
    server->add_r_device("smoke_sensor", &s_fum);
    server->add_r_device("window_sensor_1", &s_jan);
    server->add_r_device("window_sensor_2", &s_por);
    server->add_r_device("gate_in_sensor", &sc_in);
    server->add_r_device("gate_out_sensor", &sc_out);

    server->set_temp_hum_sensor(&dht22);

    server->run();

    DistributedServer::shut_down();
    return 0;
}
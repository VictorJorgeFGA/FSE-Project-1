#ifndef _DISTRIBUTED_SERVER_HPP_
#define _DISTRIBUTED_SERVER_HPP_

#include <string>
#include <sys/socket.h>

struct DistributedServer
{
    int communication_socket_fd;
    float temperature = 0.0;
    float humidity = 0.0;
    bool lamp_1 = false;
    bool lamp_2 = false;
    bool air_conditioner = false;
    bool projector = false;
    bool buzzer_alarm = false;
    bool smoke_sensor = false;
    bool presence_sensor = false;
    bool window_sensor_1 = false;
    bool window_sensor_2 = false;
    int people_amount = 0;
    std::string address;
    struct sockaddr_in distributed_server_address;
};

#endif
#ifndef _DISTRIBUTED_SERVER_HPP_
#define _DISTRIBUTED_SERVER_HPP_

#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <string.h>
#include <errno.h>

#include <map>

#include <sys/time.h>

#include "Device.hpp"

class DistributedServer
{
public:
    static void start_up(std::string t_main_server_address, int t_main_server_port);
    static DistributedServer * server();
    static void shut_down();

    static void handle_terminator_signals(int signum);
    static void read_devices(int signum);

    void run();
    void add_r_device(std::string t_device_name, Device * t_device);
    void add_w_device(std::string t_device_name, Device * t_device);

private:
    DistributedServer();
    DistributedServer(DistributedServer &);
    DistributedServer(std::string & t_main_server_address, int t_main_server_port);
    ~DistributedServer();

    void push_devices_information();

    static DistributedServer * _distributed_server;
    static bool _server_is_running;

    std::string m_main_server_ip_address;
    int m_main_server_port;
    struct sockaddr_in m_main_server_address;
    int m_communication_socket_file_descriptor;

    std::map<std::string, Device*> m_w_devices;
    std::map<std::string, Device*> m_r_devices;

    struct timeval m_last_frame;

protected:
};

#endif
#ifndef _MAIN_SERVER_HPP_
#define _MAIN_SERVER_HPP_

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "DistributedServer.hpp"

class MainServer
{
public:
    static void handle_terminator_signal(int t_signal_num);
    static void start_up(int t_port);
    static MainServer * server();
    static void shut_down();

    void run();

private:
    MainServer();
    MainServer(MainServer &);
    MainServer(int t_port);
    ~MainServer();

    void handle_user_input();
    void display_current_states();
    std::string bool_to_on_off(bool);
    void accept_new_connection();
    bool receive_message_from_distributed_server(DistributedServer &);

    static bool is_a_toggleable_device(std::string &);

    static MainServer * _main_server;
    static const int stdin = 0;
    static std::vector<std::string> _toggleable_devices;
    static char _buffer[1000];

    int m_port;
    int m_listening_socket_fd;
    struct sockaddr_in m_listening_socket_address;
    fd_set m_file_descriptors_set;
    bool m_server_is_running;
    std::vector<DistributedServer> m_distributed_servers;
    bool m_security_system;
    std::string m_ui_alert_message;

protected:
};

#endif
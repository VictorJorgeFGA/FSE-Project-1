#include "DistributedServer.hpp"

#include <iostream>
#include <cstdlib>
#include <signal.h>

DistributedServer * DistributedServer::_distributed_server = nullptr;
bool DistributedServer::_server_is_running = false;

void DistributedServer::start_up(std::string & t_main_server_address, int t_main_server_port)
{
    if (_distributed_server != nullptr) {
        std::cerr << "Attempt to initialize the distributed server twice" << std::endl;
        exit(EXIT_FAILURE);
    }

    _distributed_server = new DistributedServer(t_main_server_address, t_main_server_port);
}

DistributedServer * DistributedServer::server()
{
    if (_distributed_server == nullptr) {
        std::cerr << "Attempt to get server instance with no previous initialization" << std::endl;
        exit(EXIT_FAILURE);
    }

    return _distributed_server;
}

void DistributedServer::shut_down()
{
    if (_distributed_server == nullptr) {
        std::cerr << "Attempt to shut down the distributed server with no previous initialization" << std::endl;
        exit(EXIT_FAILURE);
    }

    delete _distributed_server;
}

void DistributedServer::handle_terminator_signals(int signum)
{
    std::cout << "Gracefully stopping..." << std::endl;
    _server_is_running = false;
}

void DistributedServer::run()
{
    _server_is_running = true;
    while (_server_is_running) {
        push_devices_information();
        sleep(1);
    }
}

void DistributedServer::add_toggleable_device(std::string && t_device_name, Device * t_device)
{
    if (m_toggleable_devices.count(t_device_name) > 0) {
        std::cerr << "\033[0;33mWARNING:\033[0m Attempt to add duplicated device: " << t_device_name << std::endl;
    }
    m_toggleable_devices[t_device_name] = t_device;
}

DistributedServer::DistributedServer(std::string & t_main_server_address, int t_main_server_port):
m_main_server_ip_address(t_main_server_address), m_main_server_port(t_main_server_port)
{
    m_communication_socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (m_communication_socket_file_descriptor == 0) {
        std::cerr << "Unable to create the communication socket file descriptor: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    m_main_server_address.sin_family = AF_INET;
    m_main_server_address.sin_addr.s_addr = inet_addr(m_main_server_ip_address.c_str());
    m_main_server_address.sin_port = htons(t_main_server_port);

    if (inet_pton(AF_INET, m_main_server_ip_address.c_str(), &m_main_server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address. Address not supported: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (connect(m_communication_socket_file_descriptor, (sockaddr *) &m_main_server_address, sizeof(m_main_server_address)) < 0) {
        std::cerr << "Unable to connect to server " << m_main_server_ip_address << ":" << m_main_server_port << ". Error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_terminator_signals);
    signal(SIGTERM, handle_terminator_signals);
    signal(SIGABRT, handle_terminator_signals);
}

DistributedServer::~DistributedServer()
{
    close(m_communication_socket_file_descriptor);
}

void DistributedServer::push_devices_information()
{
    if (!m_toggleable_devices.empty()) {
        std::string information = "";
        bool need_comma = false;
        for (auto device : m_toggleable_devices)
            information += device.first + " " + std::to_string(device.second->get_state()) + "\n";
        std::cout << information << std::endl;
        send(m_communication_socket_file_descriptor, information.c_str(), information.size(), 0);
    }
}
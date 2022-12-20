#include "DistributedServer.hpp"
#include "Environment.hpp"

#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sstream>
#include <vector>

#ifndef _DEVELOPMENT_MODE_
    #include <wiringPi.h>
#endif

DistributedServer * DistributedServer::_distributed_server = nullptr;
bool DistributedServer::_server_is_running = false;

void DistributedServer::start_up(std::string t_main_server_address, int t_main_server_port)
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

void DistributedServer::read_devices(int signum)
{
    signal(SIGALRM, read_devices);
    auto server = DistributedServer::server();
    server->increment_cycle();
    for (auto r_device : server->m_r_devices)
        r_device.second->refresh_state(server->cycles());

    for (auto w_device : server->m_w_devices)
        w_device.second->refresh_state(server->cycles());

    auto m_people_count_in = server->m_r_devices["gate_in_sensor"];
    auto m_people_count_out = server->m_r_devices["gate_out_sensor"];

    server->m_previous_people_count = server->m_people_count;
    if (m_people_count_in->previous_and_current_state_differ()) {
        server->m_people_count += m_people_count_in->get_current_state() ? 1 : 0;
        m_people_count_in->sync_previous_and_current_states();
    }

    if (m_people_count_out->previous_and_current_state_differ()) {
        server->m_people_count -= m_people_count_out->get_current_state() ? 1 : 0;
        m_people_count_out->sync_previous_and_current_states();
    }

    server->m_temp_hum_sensor->refresh_state(server->cycles());
}

void DistributedServer::run()
{
    push_devices_information(false);

    _server_is_running = true;
    signal(SIGALRM, read_devices);
    while (_server_is_running) {
        ualarm(50000, 0); // TODO remove hardcoded cycle
        pause();
        push_devices_information();
        handle_main_server_commands();
        #ifdef _NO_SOCKET_
        if (cycles() % 20 == 0) {
            system("clear");
            for (auto d : m_r_devices) {
                std::cout << d.first << " " << d.second->get_current_state() << std::endl;
            }
            for (auto d: m_w_devices) {
                std::cout << d.first << " " << d.second->get_current_state() << std::endl;
            }
        }
        #endif
    }
}

void DistributedServer::add_w_device(std::string t_device_name, Device * t_device)
{
    if (m_w_devices.count(t_device_name) > 0) {
        std::cerr << "\033[0;31mERROR:\033[0m Attempt to add duplicated write device: " << t_device_name << std::endl;
        exit(EXIT_FAILURE);
    }
    m_w_devices[t_device_name] = t_device;
}

void DistributedServer::add_r_device(std::string t_device_name, Device * t_device)
{
    if (m_r_devices.count(t_device_name) > 0) {
        std::cerr << "\033[0;31mERROR:\033[0m Attempt to add duplicated read device: " << t_device_name << std::endl;
        exit(EXIT_FAILURE);
    }
    m_r_devices[t_device_name] = t_device;
}

void DistributedServer::set_temp_hum_sensor(DHT22 * t_temp_hum_sensor)
{
    m_temp_hum_sensor = t_temp_hum_sensor;
}

void DistributedServer::increment_cycle()
{
    m_cycles = m_cycles % 2000000 + 1;
}

int DistributedServer::cycles() const noexcept
{
    return m_cycles;
}

DistributedServer::DistributedServer(std::string & t_main_server_address, int t_main_server_port):
m_main_server_ip_address(t_main_server_address), m_main_server_port(t_main_server_port), m_people_count(0), m_previous_people_count(0)
{
    #ifndef _NO_SOCKET_
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
    if (fcntl(m_communication_socket_file_descriptor, F_SETFL, fcntl(m_communication_socket_file_descriptor, F_GETFL, 0) | O_NONBLOCK) == -1) {
        std::cerr << "\033[1;31mERROR:\033[0m Unable to set socket to non-blocking mode" << std::endl;
        exit(EXIT_FAILURE);
    }
    #endif

    signal(SIGINT, handle_terminator_signals);
    signal(SIGTERM, handle_terminator_signals);
    signal(SIGABRT, handle_terminator_signals);

    std::cout << "Distributed Server started successfully!" << std::endl;
}

DistributedServer::~DistributedServer()
{
    #ifndef _NO_SOCKET_
    close(m_communication_socket_file_descriptor);
    #endif
}

void DistributedServer::push_devices_information(bool only_recently_updated)
{
    std::string information = "";
    for (auto device : m_w_devices) {
        if (!only_recently_updated || device.second->previous_and_current_state_differ()) {
            information += device.first + " " + std::to_string(device.second->get_current_state()) + "\n";
            device.second->sync_previous_and_current_states();
        }
    }
    for (auto device : m_r_devices) {
        if (!only_recently_updated || (device.second->previous_and_current_state_differ() && device.first != "gate_in_sensor" && device.first != "gate_out_sensor")) {
            information += device.first + " " + std::to_string(device.second->get_current_state()) + "\n";
            device.second->sync_previous_and_current_states();
        }
    }

    if (!only_recently_updated || m_previous_people_count != m_people_count)
        information += "people_amount " + std::to_string(m_people_count) + "\n";

    if (!only_recently_updated || m_temp_hum_sensor->previous_and_current_state_differ()) {
        information += "temp " + std::to_string(m_temp_hum_sensor->temperature()) + "\n";
        information += "hum " + std::to_string(m_temp_hum_sensor->humidity()) + "\n";
        m_temp_hum_sensor->sync_previous_and_current_states();
    }

    #ifndef _NO_SOCKET_
    if (!information.empty())
        send(m_communication_socket_file_descriptor, information.c_str(), information.size(), 0);
    #endif
}

void DistributedServer::handle_main_server_commands()
{
    memset(m_data, '\0', sizeof(m_data));
    int bytes_read = recv(m_communication_socket_file_descriptor, m_data, sizeof(m_data), 0);
    if (bytes_read == -1)
        return;
    std::string cmd(m_data);
    std::cout << "Handling command: " << cmd << std::endl;

    std::vector<std::string> split_cmd;
    std::stringstream stream_data(m_data);

    std::string tmp_str;
    while (std::getline(stream_data, tmp_str, ' '))
        split_cmd.push_back(tmp_str);
    if (split_cmd[0] == "toggle") {
        if (m_w_devices.count(split_cmd[1]) < 1) {
            std::cerr << "\033[0;31mERROR:\033[0m Unknow writable device " << split_cmd[1] << std::endl;
            return;
        }
        m_w_devices[split_cmd[1]]->toggle_state();
    }
    else {
        if (m_w_devices.count(split_cmd[0]) < 1) {
            std::cerr << "\033[0;31mERROR:\033[0m Unknow writable device " << split_cmd[0] << std::endl;
            return;
        }
        #ifndef _DEVELOPMENT_MODE_
        m_w_devices[split_cmd[0]]->set_state(split_cmd[1] == "1" ? HIGH : LOW );
        #else
        m_w_devices[split_cmd[0]]->set_state(split_cmd[1] == "1" ? 1 : 0 );
        #endif
        std::cout << "new state: " <<  m_w_devices[split_cmd[0]]->get_current_state() << std::endl;
    }
    push_devices_information(false);
    std::cout << "\033[0;32mSUCCESS\033[0m" << std::endl;
}

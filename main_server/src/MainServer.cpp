#include <iostream>
#include <cstdlib>
#include <string.h>
#include <errno.h>
#include <csignal>
#include <string>
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>

#include "MainServer.hpp"

MainServer * MainServer::_main_server = nullptr;
char MainServer::_buffer[1000];

std::vector<std::string> MainServer::_toggleable_devices = {
    "lamp01",
    "lamp02",
    "air_conditioner",
    "projector",
    "buzzer_alarm",
};

void MainServer::handle_terminator_signal(int t_signal_num)
{
    std::cout << std::endl << "Gracefully stopping..." << std::endl;
    if (_main_server != nullptr)
        _main_server->m_server_is_running = false;
}

void MainServer::start_up(int t_port)
{
    if (_main_server != nullptr) {
        std::cerr << "Attempt to initialize the main server twice" << std::endl;
        exit(EXIT_FAILURE);
    }
    _main_server = new MainServer(t_port);
}

MainServer * MainServer::server()
{
    if (_main_server == nullptr) {
        std::cerr << "Attempt to get the main server instance with no previous initialization" << std::endl;
        exit(EXIT_FAILURE);
    }
    return _main_server;
}

void MainServer::shut_down()
{
    if (_main_server == nullptr) {
        std::cerr << "Attempt to shut down the main server with no previous initialization" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::ofstream log_file;
    log_file.open("log.csv", std::ios::app);
    log_file << _main_server->m_log;
    log_file.close();

    delete _main_server;
    _main_server = nullptr;
}

void MainServer::run()
{
    m_server_is_running = true;
    display_current_states();
    while (m_server_is_running) {
        fd_set temp_file_descriptors_set = m_file_descriptors_set;
        int fds_amount = select(FD_SETSIZE, &temp_file_descriptors_set, nullptr, nullptr, nullptr);

        if (fds_amount == -1) {
            std::cerr << "Something wrong happened when listening to the file descriptors" << std::endl;
            continue;
        }

        if (FD_ISSET(m_listening_socket_fd, &temp_file_descriptors_set)) // Check for new incoming distributed servers connection requests
            accept_new_connection();

        if (FD_ISSET(stdin, &temp_file_descriptors_set))
            handle_user_input();

        std::vector<int> disconnected_distributed_servers;
        for (int i = 0; i < (int) m_distributed_servers.size(); i++) {
            DistributedServer & distributed_server = m_distributed_servers[i];
            if (FD_ISSET(distributed_server.communication_socket_fd, &temp_file_descriptors_set)) {
                if (!receive_message_from_distributed_server(distributed_server))
                    disconnected_distributed_servers.push_back(i);
            }
        } // Check for pushed messages from distributed servers

        for (auto i : disconnected_distributed_servers) {
            FD_CLR(m_distributed_servers[i].communication_socket_fd, &m_file_descriptors_set);
            m_distributed_servers.erase(m_distributed_servers.begin() + i);
        }

        check_security_system();
    }
}

MainServer::MainServer(int t_port):
m_port(t_port), m_security_system_triggered(false)
{
    std::cout << std::endl << "Initializing the main server..." << std::endl;
    // Initializing Listening Socket
    m_listening_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listening_socket_fd == 0) {
        std::cerr << "Unanble to create a listening socket file descriptor" << std::endl;
        exit(EXIT_FAILURE);
    }

    m_listening_socket_address.sin_family = AF_INET;
    m_listening_socket_address.sin_addr.s_addr = INADDR_ANY;
    m_listening_socket_address.sin_port = htons(m_port);

    if (bind(m_listening_socket_fd, (struct sockaddr *) &m_listening_socket_address, sizeof(m_listening_socket_address)) < 0) {
        std::cerr << "Unable to bind the listening socket to port " << m_port << ": " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    listen(m_listening_socket_fd, 100);

    // Initializing FD_SET
    FD_ZERO(&m_file_descriptors_set);
    FD_SET(m_listening_socket_fd, &m_file_descriptors_set);
    FD_SET(stdin, &m_file_descriptors_set); // 0 is the STDIN file descriptor

    // Initializing signals handlers
    signal(SIGABRT, MainServer::handle_terminator_signal);
    signal(SIGTERM, MainServer::handle_terminator_signal);
    signal(SIGINT, MainServer::handle_terminator_signal);

    // Security System starts disabled
    m_security_system = false;

    std::cout << "Main server initialized! (PID: " << getpid() << ")" << std::endl;
    std::cout << "Listening on port: " << m_port << std::endl;
}

MainServer::~MainServer()
{
    m_server_is_running = false;
    close(m_listening_socket_fd);
}

void MainServer::handle_user_input()
{
    m_ui_alert_message = "";
    std::string m_command_buffer;
    std::getline(std::cin, m_command_buffer);

    bool should_save_log = (m_command_buffer != "r" && m_command_buffer != "refresh");

    std::vector<std::string> split_cmd;
    std::stringstream stream_data(m_command_buffer);

    std::string tmp_str;
    while (std::getline(stream_data, tmp_str, ' '))
        split_cmd.push_back(tmp_str);

    if (m_command_buffer == "toggle securitysystem") {
        if (m_security_system) {// security system currently activated
            m_security_system = false;
            m_ui_alert_message = "\033[0;32mSUCCESS:\033[0m Security system is now " + bool_to_on_off(m_security_system);
        }
        else {
            bool can_activate_ss = true;
            for (auto distributed_server : m_distributed_servers) {
                can_activate_ss = can_activate_ss && !would_trigger_security_system(distributed_server);
            }
            if (can_activate_ss) {
                m_security_system = true;
                m_ui_alert_message = "\033[0;32mSUCCESS:\033[0m Security system is now " + bool_to_on_off(m_security_system);
            }
            else {
                m_ui_alert_message = "\033[0;31mERROR:\033[0m It was not possible to activate the security system. It may have presence in some room or open windows/doors";
            }
        }
    }
    else if(m_command_buffer == "refresh" || m_command_buffer == "r") {

    }
    else if ((int) split_cmd.size() == 3 && (split_cmd[0] == "turnin" || split_cmd[0] == "turnoff")) {
        if (is_a_toggleable_device(split_cmd[1])) {
            bool turn_in = (split_cmd[0] == "turnin");
            if (split_cmd[2] == "all") {
                for (auto distributed_server : m_distributed_servers) {
                    send_message_to_distributed_server(distributed_server, split_cmd[1] + (turn_in ? " 1" : " 0"));
                }
            }
            else if (str_is_number(split_cmd[2])) {
                int room_number = std::stoi(split_cmd[2]);
                if ((int) m_distributed_servers.size() > room_number || 0 <= room_number) {
                    send_message_to_distributed_server(m_distributed_servers[room_number], split_cmd[1] + (turn_in ? " 1" : " 0"));
                }
                else {
                    m_ui_alert_message = "\033[0;31mERROR\033[0m " + split_cmd[2] + " is not a valid room number";
                }
            }
            else {
                m_ui_alert_message = "\033[0;31mERROR:\033[0m " + split_cmd[2] + " is not a valid room specification";
            }
        }
        else {
            m_ui_alert_message = "\033[0;31mERROR\033[0m " + split_cmd[2] + " is not a toggleable device";
        }
    }
    else {
        m_ui_alert_message = "\033[0;31mERROR\033[0m: Unknown command '" + m_command_buffer + "'";
    }
    display_current_states();

    if (should_save_log) {
        auto current_time = std::chrono::system_clock::now();
        auto current_time_t = std::chrono::system_clock::to_time_t(current_time);
        std::string str_current_time(std::ctime(&current_time_t));
        str_current_time.pop_back(); // removing default \n
        m_log += str_current_time + "," + m_command_buffer + "\n";
    }
}

void MainServer::display_current_states()
{
    system("clear");
    std::cout << " ______________________________________________________________________________________________________________________________" << std::endl;
    printf("[     MAIN SERVER  | PID: %6d |  Running on Port: %6d                                                                    ]\n", getpid(), m_port);
    std::cout << " ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨" << std::endl;
    std::cout << " ________________                                                        ________________" << std::endl;
    std::cout << "| Rooms' devices |                                                      | Room's sensors |" << std::endl;
    std::cout << "|________________|______________________________________________________|________________|_____________________________________" << std::endl;
    std::cout << "|        | lamp01 | lamp02 | air_conditioner | projector | buzzer_alarm | smoke_sensor | presence_sensor | window01 | window02 |" << std::endl;
    std::cout << "|¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨|" << std::endl;
    for (int i = 0; i < (int) m_distributed_servers.size(); i++) {
        auto ds = m_distributed_servers[i];
        printf(
            "| room %d |   %s  |   %s  |       %s       |    %s    |      %s     |      %s     |       %s       |    %s   |   %s    |\n",
            i,
            bool_to_on_off(ds.lamp_1).c_str(),
            bool_to_on_off(ds.lamp_2).c_str(),
            bool_to_on_off(ds.air_conditioner).c_str(),
            bool_to_on_off(ds.projector).c_str(),
            bool_to_on_off(ds.buzzer_alarm).c_str(),
            bool_to_on_off(ds.smoke_sensor).c_str(),
            bool_to_on_off(ds.presence_sensor).c_str(),
            bool_to_on_off(ds.window_sensor_1).c_str(),
            bool_to_on_off(ds.window_sensor_2).c_str()
        );
    }
    std::cout << " ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ " << std::endl;
    std::cout << " _________________" << std::endl;
    std::cout << "| Rooms' statuses |" << std::endl;
    std::cout << "|_________________|______________________________" << std::endl;
    std::cout << "|        | people count | temperature | humidity |" << std::endl;
    std::cout << "|¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨|" << std::endl;
    int people_count = 0;
    for (int i = 0; i < (int) m_distributed_servers.size(); i++) {
        auto ds = m_distributed_servers[i];
        printf(
            "| room %d |      %2d      |   %5.1f ºC  |  %4.1f    |\n",
            i,
            ds.people_amount,
            ds.temperature,
            ds.humidity
        );
        people_count += ds.people_amount;
    }
    std::cout << " ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨" << std::endl;

    printf(" ____________________\n");
    printf("| Building statuses  |\n");
    printf("|____________________|_____\n");
    printf("| Total people count | %3d |\n", people_count);
    printf("| Security System    | %s |\n", bool_to_on_off(m_security_system).c_str());
    printf(" ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨\n");
    std::cout << " ______________________________________________________________________________________________________________________________" << std::endl;
    printf("[ %s\n", m_ui_alert_message.c_str());
    std::cout << " ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨" << std::endl;

    std::cout << "\tCommands:" << std::endl;
    std::cout << "\t\tturnin|turoff <device> <room>|all | <device> The available devices is presented on the Room's devices" << std::endl;
    std::cout << "\t\t                                  | table above. <room> the room number or all. Example: turnin lamp01 0." << std::endl << std::endl;
    std::cout << "\t\ttoggle securitysystem             | Enable the security system if it's currently disabled and vice versa." << std::endl << std::endl;
    std::cout << "\t\trefresh                           | Or simply 'r', refreshes the devices information on screen." << std::endl << std::endl;
    printf("Enter your command:\n");
}

std::string MainServer::bool_to_on_off(bool val)
{
    return val ? "\033[0;32mON\033[0m " : "\033[0;33mOFF\033[0m";
}

void MainServer::accept_new_connection()
{
    m_distributed_servers.push_back({});
    DistributedServer & new_ds = m_distributed_servers.back();
    socklen_t tmp;
    new_ds.communication_socket_fd = accept(m_listening_socket_fd, (struct sockaddr *) &new_ds.distributed_server_address, &tmp);
    FD_SET(new_ds.communication_socket_fd, &m_file_descriptors_set);
}

bool MainServer::receive_message_from_distributed_server(DistributedServer & distributed_server)
{
    memset(_buffer, '\0', sizeof(_buffer));
    int data_amout = recv(distributed_server.communication_socket_fd, _buffer, sizeof(_buffer), 0);
    if (data_amout == 0)
        return false;

    std::string received_data(_buffer);
    std::stringstream buffer_stream(received_data);
    std::string tmp_str;
    while (buffer_stream >> tmp_str) {
        if (tmp_str == "temperature")
            buffer_stream >> distributed_server.temperature;
        else if (tmp_str == "humidity")
            buffer_stream >> distributed_server.humidity;
        else if (tmp_str == "lamp01")
            buffer_stream >> distributed_server.lamp_1;
        else if (tmp_str == "lamp02")
            buffer_stream >> distributed_server.lamp_2;
        else if (tmp_str == "air_conditioner")
            buffer_stream >> distributed_server.air_conditioner;
        else if (tmp_str == "projector")
            buffer_stream >> distributed_server.projector;
        else if (tmp_str == "buzzer_alarm")
            buffer_stream >> distributed_server.buzzer_alarm;
        else if (tmp_str == "smoke_sensor")
            buffer_stream >> distributed_server.smoke_sensor;
        else if (tmp_str == "presence_sensor")
            buffer_stream >> distributed_server.presence_sensor;
        else if (tmp_str == "window_sensor_1")
            buffer_stream >> distributed_server.window_sensor_1;
        else if (tmp_str == "window_sensor_2")
            buffer_stream >> distributed_server.window_sensor_2;
        else if (tmp_str == "people_amount")
            buffer_stream >> distributed_server.people_amount;
        else if (tmp_str == "temp") {
            buffer_stream >> distributed_server.temperature;
            distributed_server.temperature /= 10.0f;
        }
        else if (tmp_str == "hum") {
            buffer_stream >> distributed_server.humidity;
            distributed_server.humidity /= 10.0f;
        }
    }
    return true;
}

void MainServer::send_message_to_distributed_server(DistributedServer & distributed_server, std::string msg)
{
    if (send(distributed_server.communication_socket_fd, msg.c_str(), msg.size(), 0) <= 0)
        m_ui_alert_message = "\033[0;31ERROR:\033[0m it was not possible to complete your command";
    else {
        m_ui_alert_message = "\033[0;32mDone\033[0m";
    }
}

void MainServer::check_security_system()
{
    m_security_system_triggered = false;
    for (auto distributed_server : m_distributed_servers) {
        if (distributed_server.smoke_sensor || (m_security_system && would_trigger_security_system(distributed_server))) {
            m_security_system_triggered = true;
            break;
        }
    }
    for (auto distributed_server : m_distributed_servers) {
        if (m_security_system_triggered && !distributed_server.buzzer_alarm)
            send_message_to_distributed_server(distributed_server, "buzzer_alarm 1");
    }
}

bool MainServer::would_trigger_security_system(DistributedServer & distributed_server)
{
    return (distributed_server.presence_sensor || distributed_server.window_sensor_1 || distributed_server.window_sensor_2);
}

bool MainServer::is_a_toggleable_device(std::string & device)
{
    return std::find(_toggleable_devices.begin(), _toggleable_devices.end(), device) != _toggleable_devices.end();
}

bool MainServer::str_is_number(const std::string & s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

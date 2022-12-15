#include <iostream>
#include <cstdlib>
#include <string.h>
#include <errno.h>
#include <csignal>
#include <string>
#include <cstdio>
#include <sstream>
#include <algorithm>

#include "MainServer.hpp"

MainServer * MainServer::_main_server = nullptr;
char MainServer::_buffer[1000];

std::vector<std::string> MainServer::_toggleable_devices = {
    "lamp01",
    "lamp02",
    "air_conditioner",
    "projector",
    "smoke_sensor",
    "presence_sensor",
    "window01",
    "window02"
};

void MainServer::handle_terminator_signal(int t_signal_num)
{
    std::cout << std::endl << "Gracefully stopping..." << std::endl;
    if (_main_server != nullptr) {
        _main_server->shut_down();
    }
    exit(EXIT_SUCCESS);
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
    }
}

MainServer::MainServer(int t_port):
m_port(t_port)
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
    std::cin >> m_command_buffer;
    std::vector<std::string> split_cmd;
    std::stringstream stream_data(m_command_buffer);

    std::string tmp_str;
    while (std::getline(stream_data, tmp_str, ' '))
        split_cmd.push_back(tmp_str);

    if (m_command_buffer == "toggle securitysystem") {
        m_security_system = !m_security_system;
        m_ui_alert_message = "Security System is now " + bool_to_on_off(m_security_system);
    }
    else if(m_command_buffer == "refresh" || m_command_buffer == "r") {}
    else if ((int) split_cmd.size() == 3) {
        if (is_a_toggleable_device(split_cmd[1])) {
            m_ui_alert_message = "Ok";
        } else {
            m_ui_alert_message = "\033[0;31mERROR\033[0m: " + split_cmd[1] + " is not a toggleable device";
        }
    }
    else {
        m_ui_alert_message = "\033[0;31mERROR\033[0m: Unknown command '" + m_command_buffer + "'";
    }
    display_current_states();
}

void MainServer::display_current_states()
{
    system("clear");
    std::cout << " _______________________________________________________________________________________________________________________" << std::endl;
    printf("[     MAIN SERVER  | PID: %6d |  Running on Port: %6d                                                             ]\n", getpid(), m_port);
    std::cout << " ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨" << std::endl;
    std::cout << " ________________                                                 ________________" << std::endl;
    std::cout << "| Rooms' devices |                                               | Room's sensors |" << std::endl;
    std::cout << "|________________|_______________________________________________|________________|_____________________________________" << std::endl;
    std::cout << "|        | lamp01 | lamp02 | air_conditioner | projector | alarm | smoke_sensor | presence_sensor | window01 | window02 |" << std::endl;
    std::cout << "|¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨|" << std::endl;
    for (int i = 0; i < (int) m_distributed_servers.size(); i++) {
        auto ds = m_distributed_servers[i];
        printf(
            "| room %d |   %s  |   %s  |       %s       |    %s    |  %s  |      %s     |       %s       |    %s   |   %s    |\n",
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
    std::cout << " ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨  " << std::endl;
    std::cout << " _________________" << std::endl;
    std::cout << "| Rooms' statuses |" << std::endl;
    std::cout << "|_________________|______________________________" << std::endl;
    std::cout << "|        | people count | temperature | humidity |" << std::endl;
    std::cout << "|¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨¨¨¨|¨¨¨¨¨¨¨¨¨¨|" << std::endl;
    int people_count = 0;
    for (int i = 0; i < (int) m_distributed_servers.size(); i++) {
        auto ds = m_distributed_servers[i];
        printf(
            "| room %d |      %d      |   %5.1f ºC  |  %2.1f    |\n",
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
    std::cout << " _______________________________________________________________________________________________________________________" << std::endl;
    printf("[ %s\n", m_ui_alert_message.c_str());
    std::cout << " ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨" << std::endl;


    std::cout << "\tCommands:" << std::endl;
    std::cout << "\t\ttoggle <device> <room>            | <device> The available devices is presented on the Room's devices" << std::endl;
    std::cout << "\t\t                                  | table above. <room> the room number. Example: toggle lamp01 0." << std::endl << std::endl;
    std::cout << "\t\ttoggle securitysystem             | Enable the security system if it's currently disabled and vice versa." << std::endl << std::endl;
    std::cout << "\t\trefresh                           | Or simply 'r', refreshes the devices information on screen." << std::endl << std::endl;
    printf("Enter your command:\n");
}

std::string MainServer::bool_to_on_off(bool val)
{
    return val ? "\033[0;32mON\033[0m " : "\033[0;33mOFF\033[0m";
}

bool MainServer::is_a_toggleable_device(std::string & device)
{
    return std::find(_toggleable_devices.begin(), _toggleable_devices.end(), device) != _toggleable_devices.end();
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
        else if (tmp_str == "lamp_1")
            buffer_stream >> distributed_server.lamp_1;
        else if (tmp_str == "lamp_2")
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
        else if (tmp_str == "gate_in_sensor") {
            int people_in;
            buffer_stream >> people_in;
            distributed_server.people_amount += people_in;
        }
        else if (tmp_str == "gate_out_sensor") {
            int people_out;
            buffer_stream >> people_out;
            distributed_server.people_amount -= people_out;
        }
    }
    return true;
}

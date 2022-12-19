#include "MainServer.hpp"
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[])
{
    MainServer::start_up(10431);
    MainServer * server = MainServer::server();

    server->run();
    server->shut_down();

    return EXIT_SUCCESS;
}
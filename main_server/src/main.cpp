#include "MainServer.hpp"
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[])
{
    MainServer::start_up(3030);
    MainServer * server = MainServer::server();

    server->run();

    return EXIT_SUCCESS;
}
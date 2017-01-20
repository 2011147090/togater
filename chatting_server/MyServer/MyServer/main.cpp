#include "tcp_server.h"
#include "redis_connector.h"

const int MAX_SESSION_COUNT = 1000;

int main()
{
    redis_connector::get_instance()->init_singleton();

    boost::asio::io_service io_service;

    tcp_server server(io_service);
    server.init(MAX_SESSION_COUNT);
    server.start();

    io_service.run();


    std::cout << "匙飘况农 立加 辆丰" << std::endl;

    return 0;
}
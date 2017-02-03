#include "tcp_server.h"
#include "redis_connector.h"
#include <boost/thread.hpp>

const int MAX_SESSION_COUNT = 100000;

int main()
{
    redis_connector::get_instance()->init_singleton();

    boost::asio::io_service io_service;

    tcp_server server(io_service);
    server.init(MAX_SESSION_COUNT);
    server.start();
    
    
    boost::thread thread1(boost::bind(&boost::asio::io_service::run, &io_service));
    boost::thread thread2(boost::bind(&boost::asio::io_service::run, &io_service));
    boost::thread thread3(boost::bind(&boost::asio::io_service::run, &io_service));
    boost::thread thread4(boost::bind(&boost::asio::io_service::run, &io_service));
    boost::thread thread5(boost::bind(&boost::asio::io_service::run, &io_service));
    boost::thread thread6(boost::bind(&boost::asio::io_service::run, &io_service));
    boost::thread thread7(boost::bind(&boost::asio::io_service::run, &io_service));
    io_service.run();

    
    std::cout << "Server Closed" << std::endl;

    return 0;
}
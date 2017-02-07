
#include <boost/thread.hpp>

#include "config.h"
#include "redis_connector.h"
#include "tcp_server.h"


int main()
{
    redis_connector::get_instance()->init_singleton();
    
    //for (int n = 0; n < 10000; n++)
    //    redis_connector::get_instance()->set("user" + std::to_string(n), "user" + std::to_string(n));

    boost::asio::io_service io_service;
    
    int SERVER_PORT;
    config::get_value("SERVER_PORT", SERVER_PORT);
    int MASTER_BUFFER_LEN;
    config::get_value("MASTER_BUFFER_LEN", MASTER_BUFFER_LEN);


    tcp_server server(io_service, SERVER_PORT, MASTER_BUFFER_LEN);


    int MAX_SESSION_COUNT;
    config::get_value("MAX_SESSION_COUNT", MAX_SESSION_COUNT);

    server.init(MAX_SESSION_COUNT);
    server.start();

    
    // There are 8 threads.
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
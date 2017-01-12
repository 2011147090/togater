/* c++ stl */
#include <iostream>
/* boost lib */
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
/* spd_logger */
#include <spdlog/spdlog.h>
/* json_spirit */
#include <json_spirit.h>
/* protocol buffer */
#include "protocol.h"
/* redis util*/
/* class */
#include "log_manager.h"
#include "channel_server.h"

int main()
{
    boost::asio::io_service io_service;
    tcp_server server(io_service);
    server.init(MAX_SESSION_COUNT);
    server.start();

    io_service.run();
    std::cout << "서버 종료" << std::endl;
    getchar();
    return 0;
}

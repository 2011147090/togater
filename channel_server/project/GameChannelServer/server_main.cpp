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
// channel_server.h 에서 사용
/* class */
#include "log_manager.h"
#include "channel_server.h"

int main()
{
    boost::asio::io_service io_service;
    /* redis connector module */
    redispp::Connection redis_conn(REDIS_SERVER_IP, REDIS_PORT, REDIS_PWD, false); 
    
    /* pakcet handle module */
    packet_handler packet_handler_main;
    
    /* user manager module */
    friends_manager friends_manager_main(redis_conn, packet_handler_main);
    match_manager match_manager_main(packet_handler_main,friends_manager_main,redis_conn);
    
    /* log manager module */

    
    /* db connector module */

    
    /* config module */

    
    /* main server */
    tcp_server server(io_service, friends_manager_main, match_manager_main, packet_handler_main); // config module , log module , db connector
    server.init(MAX_SESSION_COUNT);
    server.start();
    
    io_service.run();
    std::cout << "서버 종료" << std::endl;
    getchar();
    return 0;
}

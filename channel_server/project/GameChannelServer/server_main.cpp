#include <iostream>
/* boost lib */
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
/* spd_logger */
#include "log_manager.h"
/* protocol buffer */
#include "protocol.h"
/* redis util */
#include "redis_connector.h"
/* mysql util */
#include "db_connector.h"
/* channel server */
#include "channel_server.h"

int main()
{
    boost::asio::io_service io_service;
    boost::asio::steady_timer timer(io_service);
    /* redis connector module */
    redis_connector redis_connector_main;
    /* mysql connector module*/
    db_connector db_connector_main;
   
    /* pakcet handle module */
    packet_handler packet_handler_main;
    
    /* user manager module */
    friends_manager friends_manager_main(redis_connector_main, packet_handler_main, db_connector_main);
    match_manager match_manager_main(packet_handler_main, friends_manager_main, redis_connector_main);
    
    /* main server */
    tcp_server server(io_service, friends_manager_main, match_manager_main, packet_handler_main);
    int max_session_count;
    
    server.init();
    server.start();
    io_service.run();

    log_manager::get_instance()->get_logger()->info("[Server Terminated!]");
    getchar();
    return 0;
}

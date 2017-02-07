#pragma once
#include <map>
#include <iostream>
#include <string>
#include "server_session.h"
#include "log_manager.h"
#include "protocol.h"
#include "redis_connector.h"
#include "db_connector.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

class friends_manager
{
public:
    friends_manager(redis_connector& redis_connector, packet_handler& handler, db_connector &mysql);
    ~friends_manager();
    bool lobby_login_process(session *login_session, const char *packet, const int packet_size);
    void del_redis_token(std::string token);
    bool lobby_logout_process(session *logout_session, const char *packet, const int packet_size);
    void search_user(session * request_session, std::string target_id, friends_response & message);
    void add_friends(session *request_session, std::string target_id);
    void del_friends(session *request_session, std::string target_id);
    void process_friends_function(session *request_session, const char *packet, const int packet_size);
    session * find_id_in_user_map(std::string target_id);
    void del_id_in_user_map(std::string target_id);
    void add_id_in_user_map(session *request_session, std::string request_id);
private:
    redis_connector& redis_connector_;
    packet_handler& packet_handler_;
    std::map<std::string, session *> user_id_map_;

    boost::mutex user_id_map_mtx;
    /* data base connection */
    db_connector &db_connector_;
};
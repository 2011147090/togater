#pragma once
#include <iostream>
#include <deque>
#include "server_session.h"
#include "protocol.h"
#include "friends_manager.h"
#include "redispp.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>


class match_manager
{
public:
    match_manager(packet_handler &packet_handler, friends_manager &friends_manager, redispp::Connection &redis_connection);
    ~match_manager();
    void process_matching(session *request_session, const char *packet, const int data_size);
    void process_matching_with_friends(session *request_session, const char *packet, const int data_size);
private:
    void set_matching_que(session *request_session, rating_name request_rating);
    void get_matching_que(std::deque<session *> &target_que);

    inline std::string generate_room_key() 
    { 
        boost::uuids::uuid random_uuid = boost::uuids::random_generator()();
        char room_key[40] = { 0, };
        char *ptr = room_key;
        ptr += sprintf(ptr, "Room:");
        for (auto t = random_uuid.begin(); t != random_uuid.end(); ++t)
        {
            ptr += sprintf(ptr, "%0x", *t);
        }
        std::string ret_key = room_key;
        return ret_key;
    }
    
    unsigned int room_number;
    packet_handler &packet_handler_;
    friends_manager &friends_manager_;
    redispp::Connection &redis_connection_;

    std::deque<session *> bronze_que;
    std::deque<session *> silver_que;
    std::deque<session *> gold_que;
    std::deque<session *> platinum_que;
    std::deque<session *> diamond_que;
    std::deque<session *> master_que;
    std::deque<session *> challenger_que;
};
#pragma once
#include <iostream>
#include <deque>
#include "server_session.h"
#include "protocol.h"
#include "friends_manager.h"
#include "redispp.h"

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

    inline unsigned int generate_room_info() { return room_number = (room_number + 1) % 5000; }; //shared resource

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
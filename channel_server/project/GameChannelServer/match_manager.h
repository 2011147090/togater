#pragma once
#include "server_session.h"
#include "friends_manager.h"
#include "redis_connector.h"
#include "log_manager.h"
#include "config.h"

class match_manager
{
public:
    match_manager(packet_handler &packet_handler, friends_manager &friends_manager, redis_connector &redis_connection);
    ~match_manager();
    void process_matching(session *request_session, const char *packet, const int data_size);
    void process_matching_with_friends(session *request_session, const char *packet, const int data_size);
    void rematching_que(session *request_session);
private:
    void set_matching_que(session *request_session, rating request_rating);
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
    
    packet_handler &packet_handler_;
    friends_manager &friends_manager_;
    redis_connector &redis_connection_;

    boost::mutex rank_que_mtx[MAX_RATING];
    std::deque<session *> matching_que[MAX_RATING];
};
#pragma once
#include "preHeaders.h"
#include "singleton.h"
#include "critical_section.h"

class redis_manager : public singleton<redis_manager>, public multi_thread_sync<redis_manager>{
private:
    redispp::Connection* conn;

public:
    virtual bool init_singleton();

    redis_manager();
    ~redis_manager();
    
    bool check_room(std::string room_key);
    void remove_room_info(std::string room_key);
};
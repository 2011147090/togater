#include "redis_manager.h"

redis_manager::redis_manager()
{}

redis_manager::~redis_manager()
{}

bool redis_manager::init_singleton()
{
    thread_sync sync;

    conn = new redispp::Connection("127.0.0.1", "6379", "password", false);

    if (conn == NULL)
        return false;

    conn->set("temp", "0");

    return true;
}

bool redis_manager::check_room(std::string room_key)
{
    thread_sync sync;

    auto value = conn->get(room_key);

    if (!value.result().is_initialized())
        return false;

    int i = atoi(value.result().get().c_str());
    i++;

    char value_str[11] = "";

    itoa(i, value_str, 10);

    conn->set(room_key, value_str);

    return true;
}

void redis_manager::remove_room_info(std::string room_key)
{
    thread_sync sync;

    conn->del(room_key);
}
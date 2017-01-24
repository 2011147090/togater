#include "redis_connector.h"
#include "log_manager.h"
#include "configurator.h"

redis_connector::redis_connector() {}
redis_connector::~redis_connector() {}

bool redis_connector::init_singleton()
{
    thread_sync sync;

    std::string ip;
    configurator::get_value("redis_server_ip", ip);

    std::string port;
    configurator::get_value("redis_server_port", port);

    system_log->info("redis_connector:connect, ip:{}, port:{}", ip, port);
    conn = new redispp::Connection(ip, port, "password", false);

    if (conn == nullptr)
    {
        system_log->error("redis_connector:conn_is_null");
        return false;
    }

    conn->set("temp", "0");

    return true;
}

bool redis_connector::release_singleton() { return true; }

bool redis_connector::check_room(std::string room_key)
{
    thread_sync sync;

    auto value = conn->get(room_key);

    if (!value.result().is_initialized())
    {
        system_log->error("redis_connector:check_room, room_key_is_null");
        return false;
    }
    
    int i = atoi(value.result().get().c_str());
    i++;

    char value_str[11] = "";

    itoa(i, value_str, 10);

    conn->set(room_key, value_str);

    return true;
}

void redis_connector::remove_room_info(std::string room_key)
{
    thread_sync sync;

    if (!conn->del(room_key).result())
        system_log->error("redis_connector:remove_room_info, room_key_is_null");
}

void redis_connector::remove_player_info(std::string player_key)
{
    thread_sync sync;

    if (!conn->del(player_key).result())
        system_log->error("redis_connector:remove_player_info, player_key_is_null");
}
#include "redis_connector.h"
#include "config.h"


redis_connector::redis_connector()
{
}

redis_connector::~redis_connector()
{
}

bool redis_connector::init_singleton()
{
    std::string REDIS_IP;
    config::get_value("REDIS_IP", REDIS_IP);

    std::string REDIS_PORT;
    config::get_value("REDIS_PORT", REDIS_PORT);

    std::string REDIS_PW;
    config::get_value("REDIS_PW", REDIS_PW);


    conn_ = new redispp::Connection(REDIS_IP, REDIS_PORT, REDIS_PW, false);


    if (conn_ == nullptr)
        return false;
    
    return true;
}

bool redis_connector::release_singleton()
{
    delete conn_;


    if (conn_ != nullptr)
        return false;

    return true;
        
}

std::string redis_connector::get(std::string key)
{
    if (conn_->get(key).result().is_initialized())
        return conn_->get(key);

    return "";
}

void redis_connector::set(std::string key, std::string value)
{
    conn_->set(key, value);
}

void redis_connector::del(std::string key)
{
    conn_->del(key);
}
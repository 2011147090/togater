#include "redis_connector.h"

const std::string redis_ip = "127.0.0.1";//"192.168.1.201";
const std::string redis_port = "6379";
const std::string redis_password = "password";


redis_connector::redis_connector()
{
}

redis_connector::~redis_connector()
{
}

bool redis_connector::init_singleton()
{
    conn_ = new redispp::Connection(redis_ip, redis_port, redis_password, false);


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

void redis_connector::set(std::string key, std::string value)
{
    conn_->set(key, value);
}

std::string redis_connector::get(std::string key)
{
    if (conn_->get(key).result().is_initialized())
        return conn_->get(key);

    return "";
}
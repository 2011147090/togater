#pragma once
#include <cstdlib>
#include <iostream>
#include <mysql.h>
#include <vector>
#include <deque>
#include "protocol.h"
#include "log_manager.h"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

class db_connector
{
public:
    db_connector(MYSQL &init);
    ~db_connector();
    void init();
    bool get_query_user_info(const std::string request_id, int &win,int &lose,int &rating);
    bool get_user_friends_list(const std::string request_id, std::vector<std::string> &friends_list);
    bool add_user_frineds_list(const std::string request_id, const std::string target_id);
    bool del_user_frineds_list(const std::string request_id, const std::string target_id);
    
    MYSQL *get_connection(int &index);
    void set_connection(const int index);

private:
    void load_mysql_config();
    boost::mutex connection_mtx;
    std::deque<int> connection_que;
    std::vector<MYSQL *> connection_pool;

    MYSQL &init_instance;

    std::string ip, id, pwd, db_name;
    int pool_size, port;
};

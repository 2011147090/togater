#pragma once
#include <cstdlib>
#include <iostream>
#include <mysql.h>
#include <vector>
#include "protocol.h"

class db_connector
{
public:
    db_connector(MYSQL &init);
    ~db_connector();
    bool get_query_user_info(const std::string request_id, int &win,int &lose,int &rating);
    bool get_user_friends_list(const std::string request_id, std::vector<std::string> &friends_list);
    bool add_user_frineds_list(const std::string request_id, const std::string target_id);
    bool del_user_frineds_list(const std::string request_id, const std::string target_id);
private:
    MYSQL *mysql_connection;
    MYSQL &init_instance;
    MYSQL_RES *query_result;
    MYSQL_ROW sql_row;
    

};

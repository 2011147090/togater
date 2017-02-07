#include "db_connector.h"


db_connector::db_connector(MYSQL &init)
    : init_instance(init)
{
    mysql_init(&init_instance);
    mysql_connection = mysql_real_connect(&init_instance, MYSQL_SERVER_IP, MYSQL_ID, MYSQL_PASSWORD, MYSQL_DB_NAME, MYSQL_PORT, (char *)nullptr, 0);
    if (mysql_connection == nullptr)
    {
        std::cout << "connection error" << std::endl;
    }
}

db_connector::~db_connector()
{
    if (query_result->lengths > 0)
    {
        mysql_free_result(query_result);
    }
    mysql_close(mysql_connection);
}

bool db_connector::get_query_user_info(const std::string request_id, int & win, int & lose, int & rating)
{
    char query[100];
    sprintf(query, "SELECT * FROM user_info WHERE id='%s'", request_id.c_str());
    int query_state = mysql_query(mysql_connection, query);
    if (query_state != 0)
    {
        std::cout << "[" << query << "]" << " : ERROR" << endl;
        return false;
    }

    query_result = mysql_store_result(mysql_connection);
    if (query_result->row_count <= 0)
    {
        return false;
    }
    while ((sql_row = mysql_fetch_row(query_result)) != NULL)
    {
        win = atoi(sql_row[3]);
        lose = atoi(sql_row[4]);
        rating = atoi(sql_row[5]);
    }
    mysql_free_result(query_result);
    return true;
}

bool db_connector::get_user_friends_list(const std::string request_id, std::vector<std::string>& friends_list)
{
    char query[100];
    sprintf(query, "SELECT id_2 FROM friend_info WHERE id_1='%s'", request_id.c_str());
    int query_state = mysql_query(mysql_connection, query);
    if (query_state != 0)
    {
        std::cout << "[" << query << "]" << " : ERROR" << endl;
        return false;
    }
    query_result = mysql_store_result(mysql_connection);
    while ((sql_row = mysql_fetch_row(query_result)) != NULL)
    {
        friends_list.push_back(sql_row[0]);
    }
    mysql_free_result(query_result);
    return true;
}

bool db_connector::add_user_frineds_list(const std::string request_id, const std::string target_id)
{
    char query[100];
    sprintf(query, "INSERT INTO friend_info (id_1,id_2) VALUES ('%s','%s')", request_id.c_str(), target_id.c_str());
    int query_state = mysql_query(mysql_connection, query);
    if (query_state != 0)
    {
        return false;
    }
    return true;
}

bool db_connector::del_user_frineds_list(const std::string request_id, const std::string target_id)
{
    char query[100];
    sprintf(query, "DELETE FROM friend_info WHERE id_1='%s' AND id_2='%s'", request_id.c_str(),target_id.c_str());
    int query_state = mysql_query(mysql_connection, query);
    int row = mysql_affected_rows(mysql_connection);
    if (query_state != 0 || row == 0)
    {
        return false;
    }
    return true;
}

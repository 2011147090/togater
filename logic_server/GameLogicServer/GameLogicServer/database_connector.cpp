#include "database_connector.h"

_DB_CONNECTION::_DB_CONNECTION()
{
    connection_ = nullptr;
    sql_result_ = nullptr;
    
    query_state_ = 0;
}

bool database_connector::init_singleton()
{
    thread_sync sync;

    mysql_init(&session_.conn);

    session_.connection_ = mysql_real_connect(
        &session_.conn,
        DB_HOST,
        DB_USER,
        DB_PASS,
        DB_NAME,
        3306,
        (char*)NULL, 0
    );

    if (session_.connection_ == nullptr)
        return false;
    
    work_thread = new std::thread(&database_connector::process_queue, this);

    if (work_thread == nullptr)
        return false;

    return true;
}

bool database_connector::release_singleton()
{
    thread_sync sync;

    mysql_close(session_.connection_);

    is_work = false;

    work_thread->join();

    return true;
}

bool database_connector::push_query(db_query query)
{
    thread_sync sync;

    queue_list_.push(query);

    return true;
}

void database_connector::process_queue()
{
    is_work = true;

    while (is_work)
    {
        thread_sync sync;

        if (queue_list_.empty())
            continue;

        auto query = queue_list_.front();

        session_.query_state_ = mysql_query(
            session_.connection_,
            query.query_.c_str()
        );

        if (session_.query_state_ != 0)
        {
            int k;
        }

        session_.sql_result_ = mysql_store_result(session_.connection_);

        if (session_.sql_result_ == nullptr)
        {
            int aSD;
        }

        if (query.callback_func != nullptr)
            query.callback_event(session_.sql_result_);

        queue_list_.pop();
    }
}

db_query::db_query()
{
    callback_func = nullptr;
}

void db_query::callback_event(MYSQL_RES* sql_result)
{
    (instance_->*callback_func)(sql_result);

    if (sql_result != nullptr)
        mysql_free_result(sql_result);
}
#include "preHeaders.h"
#include "tcp_server.h"
#include "log_manager.h"
#include "redis_connector.h"
#include "logic_worker.h"
#include "configurator.h"
#include "database_connector.h"

tcp_server* server = nullptr;

BOOL WINAPI ConsolHandler(DWORD handle)
{
    switch (handle)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT: 
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    default:
        if (database_connector::get_instance()->release_singleton())
            system_log->info("release_database_manager");

        if (logic_worker::get_instance()->release_singleton())
            system_log->info("release_logic_manager");

        if (redis_connector::get_instance()->release_singleton())
            system_log->info("release_redis_manager");

        system_log->info("server_close");
        log_manager::get_instance()->release_singleton();

        if (server != nullptr)
        {
            server->end_server();
            delete server;
        }

        return false;
    }

    return false;
}

int main(int argc, char* argv[])
{
    SetConsoleCtrlHandler(ConsolHandler, true);
    
    try
    {
        if (log_manager::get_instance()->init_singleton())
        {
            log_manager::get_instance()->set_debug_mode(true);

            system_log->info("server_start");
            system_log->info("init_log_manager");
        }
                
        if (!redis_connector::get_instance()->init_singleton())
        {
            system_log->error("failed_init_redis_manager");

            throw;
        }
        else
            system_log->info("init_redis_manager");

        if (!logic_worker::get_instance()->init_singleton())
        {
            system_log->error("failed_init_logic_worker");

            throw;
        }
        else
            system_log->info("init_logic_manager");

        if (!database_connector::get_instance()->init_singleton())
        {
            system_log->info("failed_init_database_manager");
            throw;
        }
        else
            system_log->info("init_database_manager");

        boost::asio::io_service service;
        
        int port;

        if (!configurator::get_value("port", port))
        {
            system_log->error("configurator_error, get_value:{}", port);
            throw;
        }

        server = new tcp_server(service, port);
                
        boost::thread_group io_thread;
        io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &service));
        io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &service));
        io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &service));
        io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &service));
        io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &service));

        io_thread.join_all();

        getchar();
    }
    catch (std::exception& e)
    {
        server->end_server();

        if (database_connector::get_instance()->release_singleton())
            system_log->info("release_database_manager");

        if (logic_worker::get_instance()->release_singleton())
            system_log->info("release_logic_manager");

        if (redis_connector::get_instance()->release_singleton())
            system_log->info("release_redis_manager");
        
        system_log->error("{}", e.what());

        if (server != nullptr)
        {
            server->end_server();
            delete server;
        }

        system_log->info("server_close");

        log_manager::get_instance()->release_singleton();
    }
        
    return 0;
}
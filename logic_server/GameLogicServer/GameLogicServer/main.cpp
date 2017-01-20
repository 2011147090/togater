#include "preHeaders.h"
#include "tcp_server.h"
#include "log_manager.h"
#include "redis_connector.h"
#include "logic_worker.h"
#include "configurator.h"

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
        if (logic_worker::get_instance()->release_singleton())
            system_log->info("release_logic_manager");

        if (redis_connector::get_instance()->release_singleton())
            system_log->info("release_redis_manager");

        log_manager::get_instance()->release_singleton();

        return false;
    }

    return false;
}

int main(int argc, char* argv[])
{
    SetConsoleCtrlHandler(ConsolHandler, true);


    try
    {
        system_log->info("server_start");

        if (!log_manager::get_instance()->init_singleton())
        {
            system_log->error("failed_init_log_manager");

            throw;
        }
        else
            system_log->info("init_log_manager");

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

        boost::asio::io_service io_Service;
        
        int port;

        if (!configurator::get_value("port", port))
        {
            system_log->error("configurator_error, get_value:{}", port);
            throw;
        }

        tcp_server tcp_server(io_Service, port);
                
        boost::thread_group io_thread;
        io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &io_Service));
        io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &io_Service));
        io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &io_Service));
        io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &io_Service));

        io_thread.join_all();

        getchar();
    }
    catch (std::exception& e)
    {
        if (logic_worker::get_instance()->release_singleton())
            system_log->info("release_logic_manager");

        if (redis_connector::get_instance()->release_singleton())
            system_log->info("release_redis_manager");

        log_manager::get_instance()->release_singleton();

        system_log->error("{}", e.what());
    }

    system_log->info("server_close");

    return 0;
}
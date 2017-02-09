#include "pre_headers.h"
#include "tcp_server.h"
#include "log_manager.h"
#include "logic_worker.h"

tcp_server::tcp_server(boost::asio::io_service& io_service, unsigned short port)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    connected_session_list_.reserve(1000);

    wait_accept();

    count = 0;
    end = false;

    keep_arrive_thread = new std::thread(&tcp_server::check_keep_arrive, this);
}

void tcp_server::wait_accept()
{
    thread_sync sync;

    connected_session::pointer new_connection = connected_session::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->get_socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection, boost::asio::placeholders::error));
        
    connected_session_list_.emplace_back(new_connection);
}

void tcp_server::handle_accept(connected_session::pointer new_connection, const boost::system::error_code& error)
{
    thread_sync sync;

    system_log->info("accept_new_client, current_connection{}", connected_session_list_.size());

    if (!error)
    {
        new_connection->start();
        wait_accept();
    }
    else
        system_log->error(error.message());
}

void tcp_server::end_server()
{
    thread_sync sync;

    end = true;

    keep_arrive_thread->join();
}

void tcp_server::check_keep_arrive()
{
    while (!end)
    {
        Sleep(3000);

        thread_sync sync;

        for (auto iter = connected_session_list_.begin(); iter != connected_session_list_.end(); iter++)
        {
            if (!(*iter)->get_socket().is_open())
            {
                if ((*iter)->is_start_game())
                    logic_worker::get_instance()->disconnect_room((*iter)->get_room_key(), (*iter)->get_player_key());
            
                if (!(*iter)->is_safe_disconnect())
                {
                    system_log->error("keep arrive - is not safe disconnect. remove player redis cookie. key : {}", (*iter)->get_player_key());
                    
                    redis_connector::get_instance()->remove_player_info(
                        redis_connector::get_instance()->get_id((*iter)->get_player_key())
                    );

                    redis_connector::get_instance()->remove_player_info((*iter)->get_player_key());
                }

                if ((*iter)->accept_client())
                {
                    iter = connected_session_list_.erase(iter);
                    system_log->info("keep arrive - erase session");
                }
            }
        }
    }
}
#include "pre_headers.h"
#include "tcp_server.h"
#include "log.h"
#include "logic_worker.h"

tcp_server::tcp_server(boost::asio::io_service& io_service, unsigned short port)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    wait_accept();

    connected_session_list_.reserve(2000);

    end = false;

    keep_arrive_thread = new std::thread(&tcp_server::check_connected_session, this);
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

    Log::WriteLog(_T("accept_new_client, current_connection_size: %d"), connected_session_list_.size());

    if (!error)
    {
        new_connection->start();
        wait_accept();
    }
    else
    {
        Log::WriteLog(_T("%s"), error.message().c_str());
    }
}

void tcp_server::end_server()
{
    thread_sync sync;

    end = true;

    keep_arrive_thread->join();
}

void tcp_server::check_connected_session()
{
    while (!end)
    {
        Sleep(10000);

        thread_sync sync;

        for (auto iter = connected_session_list_.begin(); iter != connected_session_list_.end();)
        {
            if (!(*iter)->get_socket().is_open())
            {
                if ((*iter)->is_in_room())
                    logic_worker::get_instance()->disconnect_room((*iter)->get_room_key(), (*iter)->get_player_key());
            
                if (!(*iter)->is_safe_disconnect() && (*iter)->get_player_key() != "")
                {
                    Log::WriteLog(_T("keep alive - is not safe disconnect. remove player redis cookie. key : %s"), (*iter)->get_player_key().c_str());
                    
                    redis_connector::get_instance()->remove_player_info(
                        redis_connector::get_instance()->get_id((*iter)->get_player_key())
                    );

                    redis_connector::get_instance()->remove_player_info((*iter)->get_player_key());
                }

                if ((*iter)->accept_client())
                {
                    iter = connected_session_list_.erase(iter);
                    Log::WriteLog(_T("keep alive - erase session"));
                }
                else
                    iter++;
            }
            else
                iter++;
        }
    }
}
#include "preHeaders.h"
#include "tcp_server.h"
#include "log_manager.h"

tcp_server::tcp_server(boost::asio::io_service& io_service)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), 13))
{
    wait_accept();
}

void tcp_server::wait_accept()
{
    connected_session::pointer new_connection =
        connected_session::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->get_socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection, boost::asio::placeholders::error));

    connected_session_list_.push_back(new_connection);
}

void tcp_server::handle_accept(connected_session::pointer new_connection, const boost::system::error_code& error)
{
    system_log->info("accept_new_client, current_connection{}", connected_session_list_.size());

    if (!error)
    {
        new_connection->start();
        wait_accept();
    }
    else
        system_log->error(error.message());
}
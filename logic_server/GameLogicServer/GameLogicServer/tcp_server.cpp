#include "preHeaders.h"
#include "tcp_server.h"

tcp_server::tcp_server(boost::asio::io_service& io_service)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), 13)), accept_cut_(0)
{
    wait_accept();
}

void tcp_server::wait_accept()
{
    ++accept_cut_;
    connected_session::pointer new_connection =
        connected_session::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->get_socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection, boost::asio::placeholders::error));

    connected_session_list_.push_back(new_connection);
}

void tcp_server::handle_accept(connected_session::pointer new_Connection, const boost::system::error_code& error)
{
    if (!error)
    {
        new_Connection->start();
        wait_accept();
    }
}
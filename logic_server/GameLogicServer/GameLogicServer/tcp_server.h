#pragma once
#include "preHeaders.h"
#include "connected_session.h"
#include <vector>

class tcp_server {
private:
    tcp::acceptor acceptor_;
    std::vector<connected_session::pointer> connected_session_list_;

public:
    tcp_server(boost::asio::io_service& io_service);

private:
    void wait_accept();
    void handle_accept(connected_session::pointer new_connection, const boost::system::error_code& error);
};
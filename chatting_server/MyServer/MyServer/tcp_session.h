#pragma once

#include <iostream>

// -- boost --
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/container/deque.hpp>

#include "chat_protocol.h"


class tcp_server;

class tcp_session
{
private:
    int session_id_;
    std::string user_id_;
    boost::asio::ip::tcp::socket socket_;

    boost::array<BYTE, 1024> receive_buffer_;
    boost::container::deque<BYTE*> send_data_queue_;

    tcp_server* server_;

    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);

public:
    tcp_session(int session_id, boost::asio::io_service& io_service, tcp_server* server);
    ~tcp_session();

    int get_session_id() { return session_id_; }
    std::string get_user_id() { return user_id_; }
    void set_user_id(std::string user_id) { user_id_ = user_id; }
    boost::asio::ip::tcp::socket& get_socket() { return socket_; }

    void post_send(const bool immediate, const int size, BYTE* data);
    void post_receive();
};
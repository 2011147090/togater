#pragma once

#include <iostream>

// -- boost --
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/container/deque.hpp>

#include "chat_protocol.h"

enum user_status{
    lobby = 0,
    room = 1
};
class tcp_server;

class tcp_session
{
private:
    int session_id_;
    std::string user_id_;
    user_status status_;

    boost::asio::ip::tcp::socket socket_;

    boost::array<BYTE, 1024> receive_buffer_;
    boost::container::deque<BYTE*> send_data_queue_;

    tcp_server* server_;
    tcp_session* opponent_session_;

    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);

public:
    tcp_session(int session_id, boost::asio::io_service& io_service, tcp_server* server);
    ~tcp_session();

    int get_session_id() { return session_id_; }
    
    std::string get_user_id() { return user_id_; }
    void set_user_id(std::string user_id) { user_id_ = user_id; }
    
    user_status get_status() { return status_; }
    void set_status(user_status status) { status_ = status; }
    
    boost::asio::ip::tcp::socket& get_socket() { return socket_; }

    void post_send(const bool immediate, const int size, BYTE* data);
    void post_receive();
};
#pragma once

#include <iostream>
#include <string>

// -- boost --
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/container/vector.hpp>
#include <boost/unordered_map.hpp>

#include "tcp_session.h"


class tcp_server
{
private:
    bool is_accepting_;

    boost::asio::ip::tcp::acceptor acceptor_;
    
    boost::container::vector<tcp_session*> session_list_;
    
    boost::container::deque<int> session_queue_;
    boost::unordered_map<std::string, tcp_session*> connected_session_map_;

    bool post_accept();
    void handle_accept(tcp_session* session, const boost::system::error_code& error);

public:
    tcp_server(boost::asio::io_service& io_service);
    ~tcp_server();

    void init(const int max_session_count);
    void start();

    void close_session(const int session_id);

    void process_packet(const int session_id, const int size, BYTE* packet);
};
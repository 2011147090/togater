#pragma once

#include <iostream>
#include <string>

// -- boost --
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/container/vector.hpp>
#include <boost/unordered_map.hpp>
#include <boost/circular_buffer.hpp>

#include "tcp_session.h"

class tcp_server
{
private:
    bool is_accepting_;

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;

    boost::container::deque<int> session_queue_;
    boost::container::vector<tcp_session*> session_list_;
    boost::unordered_map<std::string, tcp_session*> connected_session_map_;

    boost::circular_buffer<boost::array<BYTE, 1024>> master_data_queue_;


    bool post_accept();
    void handle_accept(tcp_session* session, const boost::system::error_code& error);

public:
    tcp_server(boost::asio::io_service& io_service);
    ~tcp_server();

    void init(const int max_session_count);
    void start();

    void close_session(const int session_id);

    void process_packet(const int session_id, const int size, BYTE* packet);

    

    boost::asio::strand strand_accept_;
    boost::asio::strand strand_close_;
    //boost::asio::strand strand_receive_;
    //boost::asio::strand strand_send_;
};
#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <string>

#include "server_session.h"
#include "protocol.h"

class tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service);
    ~tcp_server();

    void init(const int n_max_session_count);

    void start();

    void close_session(const int n_session_id);
    void process_packet(const int n_session_id, const char *p_data);

private:
    bool post_accept();
    void handle_accept(session *p_session, const boost::system::error_code& error);

    int seq_number_;
    bool accepting_flag_;
    boost::asio::ip::tcp::acceptor acceptor_;

    std::vector< session* > session_list_;
    std::deque<int> session_queue_;
};


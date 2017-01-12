#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <string>

#include "server_session.h"
#include "protocol.h"
#include "redispp.h"

class tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service, redispp::Connection& conn);
    ~tcp_server();

    void init(const int n_max_session_count);

    void start();

    void close_session(const int n_session_id);
    void process_packet(const int n_session_id, const char *p_data);

private:
    void process_join_packet(const int n_session_id, const char * p_data, const int parse_size);
    void process_play_rank_packet(const int n_session_id, const char * p_data, const int parse_size);
    void process_play_friends_packet(const int n_session_id, const char * p_data, const int parse_size);
    void prcoess_friends_packet(const int n_session_id, const char * p_data, const int parse_size);
    void process_logout_packet(const int n_session_id, const char * p_data, const int parse_size);
    bool post_accept();
    void handle_accept(session *p_session, const boost::system::error_code& error);

    int seq_number_;
    bool accepting_flag_;
    boost::asio::ip::tcp::acceptor acceptor_;

    std::vector< session* > session_list_;
    std::deque<int> session_queue_;
    
    /* redis */
    redispp::Connection &conn_;
};


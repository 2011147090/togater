#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <string>

#include "server_session.h"
#include "friends_manager.h"
#include "match_manager.h"
#include "protocol.h"
#include "redispp.h"

class tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service, friends_manager &friends, match_manager &match, packet_handler &packet_handler);
    ~tcp_server();

    void init(const int n_max_session_count);

    void start();
    
    void close_session(const int n_session_id);
    void process_packet(const int n_session_id, const char *p_data);

    session* get_session(const int n_session_id) { return session_list_[n_session_id]; }

private:
    bool post_accept();
    void handle_accept(session *p_session, const boost::system::error_code& error);

    int seq_number_;
    bool accepting_flag_;
    boost::asio::ip::tcp::acceptor acceptor_;

    std::vector< session* > session_list_;
    std::deque<int> session_queue_;

    /* friends manager */
    friends_manager &friends_manager_;
    /* match manager */
    match_manager &match_manager_;
    /* packet handler */
    packet_handler &packet_handler_;
};


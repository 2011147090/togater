#pragma once
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "protocol.h"

class tcp_server;

class user
{
public:
    int set_user_info(int rating, int total_battle_history, int total_win, int total_lose, std::string user_id)
    {
        rating_ = rating;
        total_battle_history_ = total_battle_history;
        total_win_ = total_win;
        total_lose_ = total_lose;
        user_id_ = user_id;
    }
        
    int get_rating() { return rating_; }
    int get_total_battle_history() { return total_battle_history_; }
    int get_total_win() { return total_win_; }
    int get_total_lose() { return total_lose_; }
    const char* get_user_id() { return user_id_.c_str(); }

private:
    int rating_;
    int total_battle_history_;
    int total_win_;
    int total_lose_;
    std::string user_id_;
};

class session:public user
{
public:
    session(int session_id, boost::asio::io_service &io_service, tcp_server* p_channel_serv);
    ~session();
    
    int get_session_id() { return session_id_; }

    boost::asio::ip::tcp::socket& get_socket() { return socket_; }

    void init();

    void post_receive();

    void post_send(const bool b_immediately, const int n_size, char *p_data);

    void set_token(const int* p_token) { sprintf(token_.data(), "%d", *p_token); }
    const char* get_token()            { return token_.data(); }

private:
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);

    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);


    int session_id_;
    boost::asio::ip::tcp::socket socket_;
    
    std::array<char, MAX_RECEIVE_BUFFER_LEN> receive_buffer_;

    int packet_buffer_mark_;
    char packet_buffer_[MAX_RECEIVE_BUFFER_LEN * 2];

    std::deque<char *> send_data_queue_;

    std::array<char, TOKEN_SIZE> token_;

    tcp_server *channel_serv_;
};

#pragma once
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "protocol.h"

class tcp_server;

class user
{
public:
    inline void set_user_info(const int rating, const int battle_history, const int win, const int lose, std::string user_id)
    {
        rating_score_ = rating;
        battle_history_ = battle_history;
        win_ = win;
        lose_ = lose;
        user_id_ = user_id;
    }
        
    inline int get_rating() { return rating_score_; }
    inline int get_battle_history() { return battle_history_; }
    inline int get_win() { return win_; }
    inline int get_lose() { return lose_; }
    inline const char* get_user_id() { return user_id_.c_str(); }

private:
    int rating_score_, battle_history_ ,win_ ,lose_;
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

    void post_send(const bool immediately, const int send_data_size, char *send_data);

    inline void set_token(const char* p_token) { memcpy(token_.data(), p_token, TOKEN_SIZE); }
    inline const char* get_token()            { return token_.data(); }

    inline void set_status(status state) { stat_ = state; }
    inline status get_status() { return stat_; }

private:
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);

    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);

    std::array<char, TOKEN_SIZE> token_;
    status stat_;
    int session_id_;
    boost::asio::ip::tcp::socket socket_;
    
    std::array<char, MAX_RECEIVE_BUFFER_LEN> receive_buffer_;
    int packet_buffer_mark_;
    char packet_buffer_[MAX_RECEIVE_BUFFER_LEN * 2];

    std::deque<char *> send_data_queue_;

    tcp_server *channel_serv_;
};

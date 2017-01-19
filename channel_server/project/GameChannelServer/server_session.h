#pragma once
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "protocol.h"

class tcp_server;

class user
{
public:
    void set_user_info(int rating, std::string user_id)
    {
        rating_score = rating;
        user_id_ = user_id;
    }
        
    int get_rating() { return rating_score; }
    const char* get_user_id() { return user_id_.c_str(); }

private:
    int rating_score;
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

    void post_send(const bool immediately, const int n_size, char *p_data);

    void set_token(const char* p_token) { memcpy(token_.data(), p_token, TOKEN_SIZE); }
    const char* get_token()            { return token_.data(); }

private:
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);

    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);


    int session_id_;
    boost::asio::ip::tcp::socket socket_;
    
    std::array<char, MAX_RECEIVE_BUFFER_LEN> receive_buffer_;
    std::array<char, MAX_RECEIVE_BUFFER_LEN> send_buffer_;
    int packet_buffer_mark_;
    char packet_buffer_[MAX_RECEIVE_BUFFER_LEN * 2];

    std::deque<char *> send_data_queue_;
    
    std::array<char, TOKEN_SIZE> token_;

    tcp_server *channel_serv_;
};

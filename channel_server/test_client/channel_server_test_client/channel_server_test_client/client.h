#pragma once
#include <iostream>
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "packet.pb.h"

class client
{
public:
    client(boost::asio::io_service &io_service);
    ~client();
    void client_connect(boost::asio::ip::tcp::endpoint &endpoint);
    void post_join_message(const char *token);
    void post_logout_message(const char *token);
    void post_play_message();
    void post_send(const bool que_flag, char *send_message, int n_size);
private:
    void recv_handle(const boost::system::error_code& error, size_t bytes_transferred);
    void send_handle(const boost::system::error_code& error, size_t bytes_transferred);
    void post_recv();
    void connect_handle(const boost::system::error_code &error);
    void process_packet(char *data, int nsize);
    void process_friends(char *data, int nszie);
    void process_join(char *data, int nsize);
    void process_game(char *data, int nsize);
    void process_game_with_friends(char *data, int size);
    void process_error(char *data, int nsize);

    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::endpoint endpoint_;
    int seq_number;
    bool token_flag;
    std::array<char, 256> trans_buffer;
    std::string write_buffer;
    std::deque<char *> send_data_que;
};
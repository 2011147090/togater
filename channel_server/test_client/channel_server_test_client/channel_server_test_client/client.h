#pragma once
#include <iostream>
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "protocol.h"

class client
{
public:
    client(boost::asio::io_service &io_service, packet_handler& handler);
    ~client();
    void client_connect(boost::asio::ip::tcp::endpoint &endpoint);
    void post_join_message(const char *token);
    void post_logout_message(const char *token);
    void post_play_message();
    void post_play_with_friends_message(const char * user_id);
    void post_search_message(const char *user_id);
    void post_add_message(const char *user_id);
    void post_del_message(const char *user_id);
    void post_accept_message();
    void post_deny_message();
    void post_send(const bool que_flag, char *send_message, int n_size);
    boost::asio::ip::tcp::socket& socket() { return socket_; }

    void set_user_info(int p_battle_history, int p_win, int p_lose, int p_rating, std::string p_user_id)
    {
        battle_history = p_battle_history;
        win = p_win;
        lose = p_lose;
        rating = p_rating;
        user_id = p_user_id;
    }
    inline int get_battle_history() { return battle_history; };
    inline int get_win() { return win; };
    inline int get_lose() { return lose; };
    inline int get_rating() { return rating; };
    inline std::string get_user_id() { return user_id; };

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

    std::string user_id,target_id;
    int battle_history;
    int win;
    int lose;
    int rating;

    packet_handler &packet_handler_;
};
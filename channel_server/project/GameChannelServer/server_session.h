#pragma once
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "protocol.h"

class server;

class session
{
public:
	session(int session_id, boost::asio::io_service &io_service, server* p_channel_serv);
	~session();
	
	int get_session_id() { return m_session_id; }

	boost::asio::ip::tcp::socket& get_socket() { return m_socket; }

	void init();

	void post_receive();

	void post_send(const bool b_immediately, const int n_size, char *p_data);

	void set_token(const int* p_token) { sprintf(m_token.data(), "%d", *p_token); }
	const char* get_token()            { return m_token.data(); }

private:
	void handle_write(const boost::system::error_code& error, size_t bytes_transferred);

	void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);


	int m_session_id;
	boost::asio::ip::tcp::socket m_socket;
	
	std::array<char, MAX_RECEIVE_BUFFER_LEN> m_receive_buffer;

	int m_packet_buffer_mark;
	char m_packet_buffer[MAX_RECEIVE_BUFFER_LEN * 2];
	
	std::deque<char *> m_send_data_queue;

	std::array<char, TOKEN_SIZE> m_token;

	server *m_channel_serv;
};

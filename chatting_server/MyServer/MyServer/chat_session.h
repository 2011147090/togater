#ifndef _SERVER_SESSION_H_
#define _SERVER_SESSION_H_

#include <deque>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "chat_protocol.h"



class chat_server;

class chat_session
{
private:
	int session_id;
	boost::asio::ip::tcp::socket socket;

	std::array<BYTE, MAX_RECEIVE_BUFFER_LEN> receive_buffer;

	int packet_buffer_mark;
	BYTE packet_buffer[MAX_RECEIVE_BUFFER_LEN * 2];

	std::deque<BYTE*> send_data_queue;
	std::string name;

	chat_server* server;


	void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
	void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);

public:
	chat_session(int session_id, boost::asio::io_service& io_service, chat_server* p_server);
	~chat_session();

	int get_session_id() { return session_id; }
	boost::asio::ip::tcp::socket& get_socket() { return socket; }

	void init();
	void post_receive();
	void post_send(const bool bImmediately, const int size, BYTE* data);

	// 
	void set_name(std::string name) { this->name = name; }
	const std::string get_name() { return this->name; }
};


#endif

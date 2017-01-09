#pragma once
#include "preHeader.h"

class protocol{
private:
	tcp::socket socket_;
	bool is_connected_;
	int test_count_;
	boost::array<BYTE, BUFSIZE> recv_buf_;

public:
	protocol(boost::asio::io_service& io);

	~protocol();

	void connect();

	bool is_run();
	bool is_socket_open();

	void handle_read();
	void handle_write();
};
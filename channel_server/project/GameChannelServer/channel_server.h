#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <string>

#include "server_session.h"
#include "protocol.h"

class server
{
public:
	server(boost::asio::io_service& io_service);
	~server();

	void init(const int n_max_session_count);

	void start();

	void close_session(const int n_session_id);
	void process_packet(const int n_session_id, const char *p_data);

private:
	bool post_accept();
	void handle_accept(session *p_session, const boost::system::error_code& error);

	int m_seq_number;
	bool m_bls_accepting;
	boost::asio::ip::tcp::acceptor m_acceptor;

	std::vector< session* > m_session_list;
	std::deque<int> m_session_queue;
};


#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "chat_session.h"

class chat_server
{
private:
	int seq_number;
	bool is_accepting;

	boost::asio::ip::tcp::acceptor acceptor;

	std::vector<chat_session*> session_list;
	std::deque<int> session_queue;


	bool post_accept()
	{
		if (session_queue.empty())
		{
			is_accepting = false;
			return false;
		}

		is_accepting = true;
		int session_id = session_queue.front();

		session_queue.pop_front();

		acceptor.async_accept(session_list[session_id]->get_socket(),
			boost::bind(&chat_server::handle_accept, this,
				session_list[session_id],
				boost::asio::placeholders::error));

		return true;
	}

	void handle_accept(chat_session* session, const boost::system::error_code& error)
	{
		if (!error)
		{
			std::cout << "클라이언트 접속 성공. SessionID: " << session->get_session_id() << std::endl;

			session->init();
			session->post_receive();

			post_accept();
		}
		else
		{
			std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
		}
	}


public:
	chat_server(boost::asio::io_service& io_service)
		:acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
	{
		is_accepting = false;
	}

	~chat_server()
	{
		for (int i = 0; i < session_list.size(); i++)
		{
			if (session_list[i]->get_socket().is_open())
			{
				session_list[i]->get_socket().close();
			}

			delete session_list[i];
		}

	}

	void init(const int max_session_count)
	{
		for (int i = 0; i < max_session_count; i++)
		{
			chat_session* session = new chat_session(i, acceptor.get_io_service(), this);
			session_list.push_back(session);
			session_queue.push_back(i);

		}
	}

	void start()
	{
		std::cout << "Server Start..." << std::endl;

		post_accept();
	}

	void close_session(const int session_id)
	{
		std::cout << "Client connection closed. session id: " << session_id << std::endl;

		session_list[session_id]->get_socket().close();
		session_queue.push_back(session_id);

		if (is_accepting == false)
		{
			post_accept();
		}
	}

	void process_packet(const int session_id, BYTE* data)
	{
		PACKET_DATA* packet_data = (PACKET_DATA*)data;

		json_spirit::Value json_data;
		json_data = packet_generator::packet_to_json(data);

//		session_list[session_id]->post_send(false, packet_data->LENGTH, json_data);
	}

};
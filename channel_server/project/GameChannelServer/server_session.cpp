#include "server_session.h"
#include "log_manager.h"
#include "channel_server.h"


session::session(int session_id, boost::asio::io_service &io_service, server* p_channel_serv)
	: m_session_id(session_id)
	, m_socket(io_service)
	, m_channel_serv(p_channel_serv)
{
}

session::~session()
{
	while (m_send_data_queue.empty() == false)
	{
		delete[] m_send_data_queue.front();
		m_send_data_queue.pop_front();
	}
}

void session::init()
{
	m_packet_buffer_mark = 0;
	memset(m_token.data(), 0, TOKEN_SIZE);
}

void session::post_receive()
{
	m_socket.async_read_some(
		boost::asio::buffer(m_receive_buffer),
		boost::bind(&session::handle_receive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void session::post_send(const bool b_immediately, const int n_size, char * p_data)
{
	char* p_send_data = nullptr;

	if (b_immediately == false)
	{
		p_send_data = new char[n_size];
		memcpy(p_send_data, p_data, n_size);

		m_send_data_queue.push_back(p_send_data);
	}
	else
	{
		p_send_data = p_data;
	}

	if (b_immediately == false && m_send_data_queue.size() > 1)
	{
		return;
	}

	boost::asio::async_write(m_socket, boost::asio::buffer(p_send_data, n_size),
		boost::bind(&session::handle_write, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void session::handle_write(const boost::system::error_code & error, size_t bytes_transferred)
{
	delete[] m_send_data_queue.front();
	m_send_data_queue.pop_front();

	if (m_send_data_queue.empty() == false)
	{
		char *p_data = m_send_data_queue.front();
		packet_header *p_header = (packet_header *)p_data;
		post_send(true, p_header->size, p_data);
	}
}

void session::handle_receive(const boost::system::error_code & error, size_t bytes_transferred)
{
	if (error)
	{
		if (error == boost::asio::error::eof)
		{
			std::cout << "클라이언트와 연결이 끊어졌습니다" << std::endl; //log
		}
		else
		{
			std::cout << "error No: " << error.value() << "error Message: " << error.message() << std::endl; //log
		}
		m_channel_serv->close_session(m_session_id); //server 클래스 작성하고 주석 풀기
	}
	else
	{
		memcpy(&m_packet_buffer[m_packet_buffer_mark], m_receive_buffer.data(), bytes_transferred);

		int n_packet_data = m_packet_buffer_mark + bytes_transferred;
		int n_read_data = 0;

		while (n_packet_data > 0) 
		{
			if (n_packet_data < sizeof(packet_header))
			{
				break;
			}

			packet_header *p_header = (packet_header *)&m_packet_buffer[n_read_data];

			if (p_header->size <= n_packet_data)
			{
				m_channel_serv->process_packet(m_session_id, &m_packet_buffer[n_read_data]);

				n_packet_data -= p_header->size;
				n_read_data += p_header->size;
			}
			else
			{
				break;
			}
		}

		if (n_packet_data > 0)
		{
			char TempBuffer[MAX_RECEIVE_BUFFER_LEN] = { 0, };
			memcpy(&TempBuffer[0], &m_packet_buffer[n_read_data], n_packet_data);
			memcpy(&m_packet_buffer[0], &TempBuffer[0], n_packet_data);
		}

		m_packet_buffer_mark = n_packet_data;

		post_receive();
	}
}

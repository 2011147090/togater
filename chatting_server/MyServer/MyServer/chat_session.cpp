#pragma once

#include "chat_server.h"

chat_session::chat_session(int session_id, boost::asio::io_service& io_service, chat_server* p_server)
	:socket(io_service), session_id(session_id), server(p_server)
{
}

chat_session::~chat_session()
{
}

void chat_session::init()
{
	packet_buffer_mark = 0;
}

void chat_session::post_receive()
{
	socket.async_read_some(boost::asio::buffer(receive_buffer),
		boost::bind(&chat_session::handle_receive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
	);
}

void chat_session::post_send(const bool bImmediately, const int size, BYTE* data)
{
	BYTE* send_data = nullptr;

	if (bImmediately == false)
	{
		send_data = new BYTE[size];
		CopyMemory(send_data, data, size);

		send_data_queue.push_back(send_data);
	}
	else
	{
		send_data = data;
	}

	if (bImmediately == false && send_data_queue.size() > 1)
	{
		return;
	}

	boost::asio::async_write(socket, boost::asio::buffer(send_data, size),
		boost::bind(&chat_session::handle_write, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
	);
}

void chat_session::handle_write(const boost::system::error_code& error, size_t bytes_transferred)
{
	delete[] send_data_queue.front();
	send_data_queue.pop_front();

	if (send_data_queue.empty() == false)
	{
		BYTE* data = send_data_queue.front();

		PACKET_DATA* packet = (PACKET_DATA*)data;

		post_send(true, packet->LENGTH, data);
	}
}

void chat_session::handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (error)
	{
		if (error == boost::asio::error::eof)
		{
			std::cout << "클라이언트와 연결이 끊어졌습니다" << std::endl;
		}
		else
		{
			std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
		}

		server->close_session(session_id);
	}
	else
	{
		// 받은 데이터를 패킷 버퍼에 저장
		CopyMemory(&packet_buffer[packet_buffer_mark], receive_buffer.data(), bytes_transferred);

		int num_packet = packet_buffer_mark + bytes_transferred;
		int num_read_data = 0;

		// 받은 데이터를 모두 처리할 때까지 반복
		while (num_packet > 0)
		{
			// 남은 데이터가 패킷 헤더보다 작으면 중단
			if (num_packet < sizeof(PACKET_DATA))
			{
				break;
			}

			PACKET_DATA* packet = (PACKET_DATA*)&packet_buffer[num_read_data];

			// 처리할 수 있는 만큼 데이터가 있다면 패킷을 처리
			if (packet->LENGTH <= num_packet)
			{
				server->process_packet(session_id, &packet_buffer[num_read_data]);

				num_packet -= packet->LENGTH;
				num_read_data += packet->LENGTH;
			}
			// 처리할 수 있는 만큼이 아니면 중단
			else
			{
				break;
			}
		}

		// 남은 데이터는 m_PacketBuffer에 저장
		if (num_packet > 0)
		{
			char TempBuffer[MAX_RECEIVE_BUFFER_LEN] = { 0, };
			memcpy(&TempBuffer[0], &packet_buffer[num_read_data], num_packet);
			memcpy(&packet_buffer[0], &TempBuffer[0], num_packet);
		}

		// 남은 데이터 양을 저장하고 데이터 받기 요청
		packet_buffer_mark = num_packet;

		post_receive();
	}
}

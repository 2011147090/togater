#include <SDKDDKVer.h>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "chat_protobuf.pb.h"

using namespace google;


enum { BUFF_SIZE = 128 };

typedef struct _MESSAGE_HEADER {
	protobuf::uint32 size;
	chat_server::message_type type;
} MESSAGE_HEADER;

const int message_header_size = sizeof(MESSAGE_HEADER);

const char SERVER_IP[] = "127.0.0.1";
const unsigned short PORT_NUMBER = 31400;


class TCP_Client
{
public:
	TCP_Client(boost::asio::io_service& io_service)
		: m_io_service(io_service),
		m_Socket(io_service),
		m_nSeqNumber(0)
	{}

	void Connect(boost::asio::ip::tcp::endpoint& endpoint)
	{
		m_Socket.async_connect(endpoint,
			boost::bind(&TCP_Client::handle_connect,
				this,
				boost::asio::placeholders::error)
		);
	}


private:
	void PostWrite()
	{
		if (m_Socket.is_open() == false)
		{
			return;
		}

		/*if (m_nSeqNumber > 7)
		{
			m_Socket.close();
			return;
		}


		++m_nSeqNumber;

		char szMessage[128] = { 0, };
		sprintf_s(szMessage, 128 - 1, "%d - Send Message", m_nSeqNumber);
		m_WriteMessage = szMessage;*/
		std::string msg;

		std::cout << "우와아아> ";
		std::cin >> msg;

		chat_server::packet_chat_normal normal_message;
		normal_message.set_chat_message(msg);


		// ------------------------------------------------------------------
		
		MESSAGE_HEADER header;

		header.size = normal_message.ByteSize();
		header.type = chat_server::normal;

		int buf_size = 0;
		buf_size = message_header_size + header.size/*normal_message.ByteSize()*/;

		CopyMemory(m_SendBuffer.begin(), (void*)&header, message_header_size);
		normal_message.SerializeToArray(m_SendBuffer.begin() + message_header_size, header.size);
		
		// ------------------------------------------------------------------


		boost::asio::async_write(m_Socket, boost::asio::buffer(m_SendBuffer),
			boost::bind(&TCP_Client::handle_write, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)
		);


		PostReceive();
	}

	void PostReceive()
	{
		//CopyMemory(&m_ReceiveBuffer, '\0', sizeof(m_ReceiveBuffer));

		m_Socket.async_read_some(boost::asio::buffer(m_ReceiveBuffer),
			boost::bind(&TCP_Client::handle_receive, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)
		);

	}


	void handle_connect(const boost::system::error_code& error)
	{
		if (error)
		{
			std::cout << "connect failed : " << error.message() << std::endl;
		}
		else
		{
			std::cout << "connected" << std::endl;

			PostWrite();
		}
	}

	void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/)
	{
	}

	void handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
	{
		if (error)
		{
			if (error == boost::asio::error::eof)
			{
				std::cout << "서버와 연결이 끊어졌습니다" << std::endl;
			}
			else
			{
				std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
			}
		}
		else
		{
			/*const std::string strRecvMessage = m_ReceiveBuffer.data();
			std::cout << "서버에서 받은 메시지: " << strRecvMessage << ", 받은 크기: " << bytes_transferred << std::endl;*/

			boost::array<BYTE, BUFF_SIZE> temp_buf = m_ReceiveBuffer;

			protobuf::io::ArrayInputStream input_array_stream(temp_buf.c_array(), temp_buf.size());
			protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

			MESSAGE_HEADER message_header;
			input_coded_stream.ReadRaw(&message_header, message_header_size);

			const void* payload_ptr = NULL;
			int remainSize = 0;
			input_coded_stream.GetDirectBufferPointer(&payload_ptr, &remainSize);

			protobuf::io::ArrayInputStream payload_array_stream(payload_ptr, message_header.size);
			protobuf::io::CodedInputStream payload_input_stream(&payload_array_stream);

			input_coded_stream.Skip(message_header.size);

			switch (message_header.type)
			{
			case chat_server::normal:
			{
				chat_server::packet_chat_normal normal_message;

				normal_message.ParseFromCodedStream(&payload_input_stream);
				std::cout << normal_message.chat_message() << std::endl;
			}
				break;
			case chat_server::whisper:
				break;
			case chat_server::notice:
				break;
			}

			PostWrite();
		}
	}



	boost::asio::io_service& m_io_service;
	boost::asio::ip::tcp::socket m_Socket;

	int m_nSeqNumber;
	boost::array<BYTE, 128> m_ReceiveBuffer;
	boost::array<BYTE, 128> m_SendBuffer;
	std::string m_WriteMessage;
};



int main()
{
	boost::asio::io_service io_service;

	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(SERVER_IP), PORT_NUMBER);

	TCP_Client client(io_service);

	client.Connect(endpoint);

	io_service.run();


	std::cout << "네트웍 접속 종료" << std::endl;

	getchar();
	return 0;
}

#include <SDKDDKVer.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <list>
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


class Session
{
public:
	Session(boost::asio::io_service& io_service)
		: m_Socket(io_service)
	{
	}

	boost::asio::ip::tcp::socket& Socket()
	{
		return m_Socket;
	}

	void PostReceive()
	{
		//memset(&m_ReceiveBuffer, '\0', sizeof(m_ReceiveBuffer));

		m_Socket.async_read_some
		(
			boost::asio::buffer(m_ReceiveBuffer),
			boost::bind(&Session::handle_receive, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)

		);

	}


private:
	void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/)
	{
	}

	void handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
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
		}
		else
		{
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

				//chat_server::packet_chat_normal normal_message;
				//boost::array<BYTE, BUFF_SIZE> output_buf;

				//protobuf::io::ArrayOutputStream output_array_stream(output_buf.c_array(), output_buf.size());
				//protobuf::io::CodedOutputStream output_coded_stream(&output_array_stream);

				//MESSAGE_HEADER output_message;
				//output_coded_stream.WriteRaw(&output_message, sizeof(MESSAGE_HEADER));
				//normal_message.SerializeToCodedStream(&output_coded_stream);
				//std::cout << normal_message.chat_message() << std::endl;

				boost::asio::async_write(m_Socket, boost::asio::buffer(temp_buf),
					boost::bind(&Session::handle_write, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred)
				);
			}
				break;
			case chat_server::whisper:
				break;
			case chat_server::notice:
				break;
			}

			/*const std::string strRecvMessage = m_ReceiveBuffer.data();
			std::cout << "클라이언트에서 받은 메시지: " << strRecvMessage << ", 받은 크기: " << bytes_transferred << std::endl;

			char szMessage[128] = { 0, };
			sprintf_s(szMessage, 128 - 1, "Re:%s", strRecvMessage.c_str());
			m_WriteMessage = szMessage;*/
			
			/*boost::asio::async_write(m_Socket, boost::asio::buffer(temp_buf),
				boost::bind(&Session::handle_write, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)
			);*/


			PostReceive();
		}
	}

	boost::asio::ip::tcp::socket m_Socket;
	std::string m_WriteMessage;
	boost::array<BYTE, BUFF_SIZE> m_ReceiveBuffer;
};

const unsigned short PORT_NUMBER = 31400;

class TCP_Server
{
public:
	TCP_Server(boost::asio::io_service& io_service)
		: m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
	{
		m_pSession = nullptr;
		StartAccept();
	}

	~TCP_Server()
	{
		if (m_pSession != nullptr)
		{
			delete m_pSession;
		}
	}

private:
	void StartAccept()
	{
		std::cout << "클라이언트 접속 대기....." << std::endl;

		m_pSession = new Session(m_acceptor.get_io_service());

		m_acceptor.async_accept(m_pSession->Socket(),
			boost::bind(&TCP_Server::handle_accept,
				this,
				m_pSession,
				boost::asio::placeholders::error)
		);
	}

	void handle_accept(Session* pSession, const boost::system::error_code& error)
	{
		if (!error)
		{
			std::cout << "클라이언트 접속 성공" << std::endl;

			pSession->PostReceive();
		}
	}

	int m_nSeqNumber;
	boost::asio::ip::tcp::acceptor m_acceptor;
	Session* m_pSession;
};

int main()
{
	boost::asio::io_service io_service;

	TCP_Server server(io_service);

	io_service.run();


	std::cout << "네트웍 접속 종료" << std::endl;

	getchar();
	return 0;
}
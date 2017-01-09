#include "preHeaders.h"
#include "connected_session.h"
#include "log_manager.h"

std::string make_daytime_string()
{
	using namespace std;
	time_t now = time(0);
	return ctime(&now);
}

connected_session::connected_session(boost::asio::io_service& io_service) : socket_(io_service), valied_key_(-1)
{}

bool connected_session::handle_check_keep_alive()
{
	boost::system::error_code error;
	//boost::asio::async_write(socket_, boost::asio::buffer(send_buf_), error);

	if (error)
		return false;

	return true;
}

void connected_session::handle_send(logic_server::message_type msg_type, const protobuf::Message& message)
{
	MESSAGE_HEADER header;
	header.size = message.ByteSize();
	header.type = msg_type;

	int buf_size = 0;
	buf_size = message_header_size + message.ByteSize();

	CopyMemory(send_buf_.begin(), (void*)&header, message_header_size);

	message.SerializeToArray(send_buf_.begin() + message_header_size, header.size);

	socket_.write_some(boost::asio::buffer(send_buf_));
}

void connected_session::handle_read(const boost::system::error_code& error, size_t /*bytes_transferred*/)
{
	if (!error)
	{
		socket_.async_read_some(boost::asio::buffer(recv_buf_),
			boost::bind(&connected_session::handle_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));

		protobuf::io::ArrayInputStream input_array_stream(recv_buf_.c_array(), recv_buf_.size());
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
		case logic_server::ENTER_REQ:
			{
				logic_server::packet_enter_req message;

				if (false == message.ParseFromCodedStream(&payload_input_stream))
					break;

				process_packet_enter_req(message);
			}
			break;

		case logic_server::PROCESS_TURN_ANS:
			{
				logic_server::packet_process_turn_ans message;

				if (false == message.ParseFromCodedStream(&payload_input_stream))
					break;

				process_packet_process_turn_ans(message);
			}
			break;
		}
	}
	else
		log->info("handle_read_error, {}", error.message());
}

connected_session::pointer connected_session::create(boost::asio::io_service& io_service)
{
	return connected_session::pointer(new connected_session(io_service));
}

tcp::socket& connected_session::get_socket()
{
	return socket_;
}

void connected_session::start()
{
	socket_.async_read_some(boost::asio::buffer(recv_buf_),
		boost::bind(&connected_session::handle_read, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}
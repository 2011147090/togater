#pragma once
#include "preHeaders.h"
#include "logic_server.pb.h"

using namespace google;

typedef struct _MESSAGE_HEADER {
	protobuf::uint32 size;
	logic_server::message_type type;
} MESSAGE_HEADER;

const int message_header_size = sizeof(MESSAGE_HEADER);

class connected_session : public boost::enable_shared_from_this<connected_session>
{
private:
	tcp::socket socket_;

	boost::array<BYTE, BUFSIZE> recv_buf_;
	boost::array<BYTE, BUFSIZE> send_buf_;

	int valied_key_;

private:
	connected_session(boost::asio::io_service& io_service);

	//void handle_accept(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/);
	void handle_read(const boost::system::error_code& error, size_t /*bytes_transferred*/);
	bool handle_check_keep_alive();
	
	void process_packet_enter_req(logic_server::packet_enter_req packet);
	void process_packet_process_turn_ans(logic_server::packet_process_turn_ans packet);
	
public:
	typedef boost::shared_ptr<connected_session> pointer;

	static pointer create(boost::asio::io_service& io_service);

	void handle_send(logic_server::message_type msg_type, const protobuf::Message& message);

	tcp::socket& get_socket();

	void start();
};
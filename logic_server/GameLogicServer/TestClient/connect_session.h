#pragma once
#include "preHeader.h"

typedef struct _MESSAGE_HEADER {
	protobuf::uint32 size;
	logic_server::message_type type;
} MESSAGE_HEADER;

const int message_header_size = sizeof(MESSAGE_HEADER);

class connect_session{
private:
	tcp::socket socket_;
	bool is_connected_;
	int test_count_;
	boost::array<BYTE, BUFSIZE> recv_buf_;
	boost::array<BYTE, BUFSIZE> send_buf_;

	int total_money_;
	int batting_money_;
	bool wait_turn;

public:
	connect_session(boost::asio::io_service& io);

	~connect_session();

	void connect();

	bool is_run();
	bool is_socket_open();

	void handle_read();
	void handle_write();
	void handle_send(logic_server::message_type msg_type, const protobuf::Message& message);

private:
	void process_packet_enter_ans(logic_server::packet_enter_ans packet);
	void process_packet_process_turn_req(logic_server::packet_process_turn_req packet);
	void process_packet_process_turn_ntf(logic_server::packet_process_turn_ntf packet);
	void process_packet_game_state_ntf(logic_server::packet_game_state_ntf packet);
};
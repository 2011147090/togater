#include "preHeader.h"
#include "connect_session.h"

connect_session::connect_session(boost::asio::io_service& io) : socket_(io)
{
	is_connected_ = false;
	test_count_ = 0;
}

connect_session::~connect_session() {}

void connect_session::connect()
{
	tcp::resolver resolver(socket_.get_io_service());
	tcp::resolver::query query(LOGIC_SERVER_IP, "daytime");

	auto endPointIter = resolver.resolve(query);
	tcp::resolver::iterator end;

	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error && endPointIter != end)
	{
		socket_.close();
		socket_.connect(*endPointIter++, error);
	}

	if (error)
		throw boost::system::system_error(error);

	is_connected_ = true;
}

bool connect_session::is_run() 
{ 
	return is_connected_; 
}

bool connect_session::is_socket_open()
{
	if (!socket_.is_open() && is_connected_)
	{
		is_connected_ = false;
		return false;
	}

	return true;
}

void connect_session::handle_read()
{
	while (is_connected_)
	{
		if (!is_socket_open())
			break;

		socket_.receive(boost::asio::buffer(recv_buf_));

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
		case logic_server::ENTER_ANS:
		{
			logic_server::packet_enter_ans message;

			if (false == message.ParseFromCodedStream(&payload_input_stream))
				break;

			process_packet_enter_ans(message);
		}
		break;

		case logic_server::PROCESS_TURN_REQ:
		{
			logic_server::packet_process_turn_req message;

			if (false == message.ParseFromCodedStream(&payload_input_stream))
				break;

			process_packet_process_turn_req(message);
		}
		break;

		case logic_server::GAME_STATE_NTF:
		{
			logic_server::packet_game_state_ntf message;

			if (false == message.ParseFromCodedStream(&payload_input_stream))
				break;

			process_packet_game_state_ntf(message);
		}
		break;
		}
		
	}
}

void connect_session::handle_send(logic_server::message_type msg_type, const protobuf::Message& message)
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

void connect_session::handle_write()
{
	int player_key;
	int room_key;
	int money;
	
	std::cout << "enter room key : " << std::endl;
	std::cin >> room_key;

	std::cout << "enter player key : " << std::endl;
	std::cin >> player_key;

	logic_server::packet_enter_req enter_req_packet;
	enter_req_packet.set_room_key(room_key);
	enter_req_packet.set_player_key(player_key);

	this->handle_send(logic_server::ENTER_REQ, enter_req_packet);

	while (is_socket_open())
	{
		while (!wait_turn) {}

		std::cout << "enter turn money key : " << std::endl;
		std::cin >> money;

		logic_server::packet_process_turn_ans process_turn_packet;
		process_turn_packet.set_money(money);
		process_turn_packet.set_player_key(player_key);

		this->handle_send(logic_server::PROCESS_TURN_ANS, process_turn_packet);
	}
}

//
//
//json_spirit::Object input_baseobj;
//input_baseobj.push_back(json_spirit::Pair("paket_version", 0));
//input_baseobj.push_back(json_spirit::Pair("protocol_type", 100));
//input_baseobj.push_back(json_spirit::Pair("key", 0));
//input_baseobj.push_back(json_spirit::Pair("room_key", 0));
//
//json_spirit::Value data(input_baseobj);
//
//int length = 0;
//
//BYTE* buf = packet_generator::json_to_packet(data, &length);


//auto buf_json = packet_generator::packet_to_json(recv_buf_.c_array());

//json_spirit::Object root_obj = buf_json.get_obj();

//if (root_obj[0].value_.get_int() != 0)
//{
//	// ³ë ¹öÁ¯
//	return;
//}

//switch (root_obj[1].value_.get_int()) {
//case PACKET_PROTOCOL::ENTER_ANS:
//	std::cout << "get packet : PACKET_PROTOCOL::ENTER_ANS" << std::endl;
//	break;
//}
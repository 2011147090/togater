#pragma once
#include "preHeaders.h"

//auto buf_json = packet_generator::packet_to_json(recv_buf_.c_array());

//json_spirit::Object root_obj = buf_json.get_obj();

//if (root_obj[0].value_.get_int() != 0)
//{
//	// ³ë ¹öÁ¯
//	return;
//}

//json_spirit::Object send_json;
//json_spirit::Value data;
//BYTE* buf = NULL;
//int length = 0;
//boost::system::error_code error;

//switch (root_obj[1].value_.get_int()) {
//case PACKET_PROTOCOL::ENTER_REQ:
//	send_json.push_back(json_spirit::Pair("paket_version", 0));
//	send_json.push_back(json_spirit::Pair("protocol_type", 101));
//	send_json.push_back(json_spirit::Pair("key", 3));
//	send_json.push_back(json_spirit::Pair("return_type", 0));

//	data = json_spirit::Value(send_json);

//	length = 0;

//	buf = packet_generator::json_to_packet(data, &length);

//	boost::asio::async_write(socket_, boost::asio::buffer(buf, length), 
//		boost::bind(&connected_session::handle_write, shared_from_this(),
//		boost::asio::placeholders::error,
//		boost::asio::placeholders::bytes_transferred));
//	break;

//case PACKET_PROTOCOL::SUBMIT_CARD_REQ:
//	break;
//}

//if (buf != NULL)
//	delete[] buf;;

//typedef struct _PACKET_DATA {
//	int LENGTH;
//	std::string DATA;
//} PACKET_DATA;
//
//class packet_generator {
//public:
//	static json_spirit::Value packet_to_json(BYTE* packet);
//	static BYTE* json_to_packet(json_spirit::Value json_data, int* length);
//};
//
//enum PACKET_PROTOCOL {
//	ENTER_REQ = 100,
//	ENTER_ANS = 101,
//	START_GAME_NTF = 102,
//	SUBMIT_CARD_REQ = 103,
//	SUBMIT_CARD_ANS = 104,
//	END_GAME_NTF = 105,
//};
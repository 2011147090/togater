#ifndef _CHAT_PROTOCOL_H_
#define _CHAT_PROTOCOL_H_

#include <string>
#include <boost/asio.hpp>
#include <json_spirit.h>


const int PORT_NUMBER = 8000;
const int MAX_RECEIVE_BUFFER_LEN = 1024;


typedef struct _PACKET_DATA {
	int LENGTH;
	std::string DATA;
} PACKET_DATA;

class packet_generator {
public:
	static json_spirit::Value packet_to_json(BYTE* packet);
	static BYTE* json_to_packet(json_spirit::Value json_data, int* length);
};

#endif
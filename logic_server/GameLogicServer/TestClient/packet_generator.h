#pragma once
#include "preHeader.h"

typedef struct _PACKET_DATA {
	int LENGTH;
	std::string DATA;
} PACKET_DATA;

class packet_generator {
public:
	static json_spirit::Value packet_to_json(BYTE* packet);
	static BYTE* json_to_packet(json_spirit::Value json_data, int* length);
};

enum PACKET_PROTOCOL {
	ENTER_REQ = 100,
	ENTER_ANS = 101,
	START_GAME_NTF = 102,
	SUBMIT_CARD_REQ = 103,
	SUBMIT_CARD_ANS = 104,
	END_GAME_NTF = 105,
};
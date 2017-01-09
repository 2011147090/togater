#include "packet_generator.h"




//json_spirit::Value packet_generator::packet_to_json(BYTE* packet)
//{
//	PACKET_DATA packet_data;
//	char temp[512];
//	CopyMemory(&packet_data.LENGTH, packet, sizeof(int));
//	CopyMemory(&temp, packet + sizeof(int), packet_data.LENGTH);
//
//	packet_data.DATA = temp;
//
//	json_spirit::Value output;
//	json_spirit::read(packet_data.DATA, output);
//
//	return output;
//}
//
//BYTE* packet_generator::json_to_packet(json_spirit::Value json_data, int* length)
//{
//	std::string data = json_spirit::write(json_data);
//
//	int str_length = data.length();
//	BYTE* packet = new BYTE[sizeof(int) + str_length + 1];
//
//	CopyMemory(packet, &str_length, sizeof(int));
//	CopyMemory(packet + sizeof(int), data.c_str(), str_length + 1);
//
//	*length = sizeof(int) + str_length + 1;
//
//	return packet;
//}
//
//#include "json_spirit.h"
//#include <string>
//
//void asd()
//{
//	json_spirit::wObject input_baseobj;
//	input_baseobj.push_back(json_spirit::wPair(L"serial", 10));    // 정수
//	input_baseobj.push_back(json_spirit::wPair(L"account", L"javawork"));    // 문자열
//
//	json_spirit::wObject input_guildobj;
//	input_guildobj.push_back(json_spirit::wPair(L"serial", 51));
//	input_guildobj.push_back(json_spirit::wPair(L"name", L"arms"));
//	input_guildobj.push_back(json_spirit::wPair(L"grade", 3));
//	input_baseobj.push_back(json_spirit::wPair(L"guild", input_guildobj));    // Object타입
//
//	json_spirit::wArray input_buddyarray;
//	input_buddyarray.push_back(20);
//	input_buddyarray.push_back(21);
//	input_buddyarray.push_back(25);
//	input_baseobj.push_back(json_spirit::wPair(L"buddy", input_buddyarray));    // Array타입
//
//	json_spirit::wValue input_Value(input_baseobj);
//	std::wstring result = json_spirit::write(input_Value);    // 문자열로 Export
//}
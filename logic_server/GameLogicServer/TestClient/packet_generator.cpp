#include "packet_generator.h"

json_spirit::Value packet_generator::packet_to_json(BYTE* packet)
{
	PACKET_DATA packet_data;
	char temp[512];
	CopyMemory(&packet_data.LENGTH, packet, sizeof(int));
	CopyMemory(&temp, packet + sizeof(int), packet_data.LENGTH);

	packet_data.DATA = temp;

	json_spirit::Value output;
	json_spirit::read(packet_data.DATA, output);

	return output;
}

BYTE* packet_generator::json_to_packet(json_spirit::Value json_data, int* length)
{
	std::string data = json_spirit::write(json_data);

	int str_length = data.length();
	BYTE* packet = new BYTE[sizeof(int) + str_length + 1];

	CopyMemory(packet, &str_length, sizeof(int));
	CopyMemory(packet + sizeof(int), data.c_str(), str_length + 1);

	*length = sizeof(int) + str_length + 1;

	return packet;
}
#include "connect_session.h"

void connect_session::process_packet_enter_ans(logic_server::packet_enter_ans packet)
{
	logic_server::packet_enter_ans recevie_packet;
	
	std::cout << "receive packet_enter_ans : " << packet.result() << std::endl;

	wait_turn = false;
}

void connect_session::process_packet_process_turn_req(logic_server::packet_process_turn_req packet)
{
	money_ = packet.money();

	std::cout << "receive packet_enter_ans : (money)" << packet.money() << std::endl;
	std::cout << "receive packet_enter_ans : (card_number)" << packet.card_number() << std::endl;

	wait_turn = true;
}

void connect_session::process_packet_game_state_ntf(logic_server::packet_game_state_ntf packet)
{
	if (packet.state() == 1)
		std::cout << "receive packet_game_state_ntf : start" << std::endl;
}
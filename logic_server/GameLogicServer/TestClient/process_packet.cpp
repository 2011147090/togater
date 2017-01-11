#include "connect_session.h"

void connect_session::process_packet_enter_ans(logic_server::packet_enter_ans packet)
{
	logic_server::packet_enter_ans recevie_packet;
	
	//std::cout << "receive packet_enter_ans : " << packet.result() << std::endl;
}

void connect_session::process_packet_process_turn_req(logic_server::packet_process_turn_req packet)
{
	batting_money_ = packet.total_money();

//	std::cout << "All batting total money :" << packet.total_money() << std::endl;

	wait_turn = true;
}

void connect_session::process_packet_process_turn_ntf(logic_server::packet_process_turn_ntf packet)
{
	std::cout << "[new turn!!]" << std::endl;

	total_money_ = packet.money();

	std::cout << "public_card_num (0)  : " << packet.public_card_number_1() << std::endl;
	std::cout << "public_card_num (1)  : " << packet.public_card_number_2() << std::endl;
	std::cout << "opponent card number : " << packet.opponent_card_number() << std::endl;
	std::cout << "my total money       : " << packet.money() << std::endl;
}


void connect_session::process_packet_game_state_ntf(logic_server::packet_game_state_ntf packet)
{
	if (packet.state() == 1)
		std::cout << "receive packet_game_state_ntf : start" << std::endl;
}
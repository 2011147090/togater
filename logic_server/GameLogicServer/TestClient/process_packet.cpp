#include "connect_session.h"

void connect_session::process_packet_enter_ans(logic_server::packet_enter_ans packet)
{
	logic_server::packet_enter_ans recevie_packet;
	
	std::cout << "recevie_packet.result() : " << std::endl;
}

void connect_session::process_packet_submit_card_req(logic_server::packet_submit_card_req packet)
{

}

void connect_session::process_packet_game_state_ntf(logic_server::packet_game_state_ntf packet)
{

}
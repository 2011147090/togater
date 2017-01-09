#include "connected_session.h"
#include "logic_worker.h"

void connected_session::process_packet_enter_req(logic_server::packet_enter_req packet)
{
	logic_server::packet_enter_ans recevie_packet;

	if (logic_worker::get_instance()
		->enter_room_player(this, packet.room_key(), packet.player_key()))
		recevie_packet.set_result(1);
	else
		recevie_packet.set_result(0);

	recevie_packet.SerializeToArray(send_buf_.c_array(), recevie_packet.ByteSize());

	handle_send(logic_server::ENTER_ANS, recevie_packet);
}

void connected_session::process_packet_process_turn_ans(logic_server::packet_process_turn_ans packet)
{
	if (!logic_worker::get_instance()->process_turn(packet.player_key(), packet.money()))
		return;
}
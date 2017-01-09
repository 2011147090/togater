#include "logic_worker.h"

logic_worker::logic_worker() : logic_thread_(NULL)
{
	InitializeCriticalSection(&critical_section_);
}

logic_worker::~logic_worker() 
{
	DeleteCriticalSection(&critical_section_);
}

bool logic_worker::init_singleton()
{
	if (logic_thread_ == NULL)
		logic_thread_ = new boost::thread(&logic_worker::process_queue, this);

	return true;
}

bool logic_worker::create_room(connected_session* session, int room_key, int player_key)
{
	EnterCriticalSection(&critical_section_);

	if (!redis_manager::get_instance()->check_room(room_key))
	{
		LeaveCriticalSection(&critical_section_);
		return false;
	}

	ROOM_INFO room_info;
	
	room_info.player_1_.key_ = player_key;
	room_info.player_1_.session_ = session;
	room_info.player_1_.total_money_ = 10;
	room_info.player_1_.money_ = 0;
	room_info.ready_player_num_ = 1;
	room_info.room_key_ = room_key;
	room_info.time_ = 0;
	room_info.turn_count_ = 0;
	room_info.state_ = ROOM_INFO::READY;

	room_list_.push_back(room_info);

	room_info.match_log_->info("create_room, room_key:{}", room_key);

	LeaveCriticalSection(&critical_section_);

	return true;
}

bool logic_worker::enter_room_player(connected_session* session, int room_key, int player_key)
{
	EnterCriticalSection(&critical_section_);
	
	for (auto iter = room_list_.begin(); iter != room_list_.end(); iter++)
	{
		if ((*iter).room_key_ == room_key)
		{
			(*iter).ready_player_num_++;
			(*iter).player_2_.total_money_ = 10;
			(*iter).player_2_.money_ = 0;
			(*iter).player_2_.key_ = player_key;
			(*iter).player_2_.session_ = session;

			(*iter).match_log_->info("enter_room_player, room_key:{}, player_key:{}", player_key);

			LeaveCriticalSection(&critical_section_);
			return true;
		}
	}

	if (create_room(session, room_key, player_key))
	{
		LeaveCriticalSection(&critical_section_);
		return true;
	}

	LeaveCriticalSection(&critical_section_);

	return false;
}

bool logic_worker::process_turn(int player_key, int money)
{
	EnterCriticalSection(&critical_section_);

	for (auto iter = room_list_.begin(); iter != room_list_.end(); iter++)
	{
		if ((*iter).player_1_.key_ == player_key)
		{
			(*iter).player_1_.submit_card_ = true;
			(*iter).player_1_.money_ = money;

			LeaveCriticalSection(&critical_section_);

			return true;
		}
		else if ((*iter).player_2_.key_ == player_key)
		{
			(*iter).player_2_.submit_card_ = true;
			(*iter).player_2_.money_ = money;

			LeaveCriticalSection(&critical_section_);

			return true;
		}
	}

	LeaveCriticalSection(&critical_section_);

	return false;
}

void logic_worker::process_queue()
{
	while (true)
	{
		EnterCriticalSection(&critical_section_);

		for (auto iter = room_list_.begin(); iter != room_list_.end(); iter++)
		{
			switch ((*iter).state_)
			{
			case ROOM_INFO::READY:
			{
				if ((*iter).ready_player_num_ == 2)
				{
					(*iter).state_ = ROOM_INFO::START;

					logic_server::packet_game_state_ntf start_ntf_packet;

					start_ntf_packet.set_state(1);

					(*iter).player_1_.session_->handle_send(logic_server::GAME_STATE_NTF, start_ntf_packet);
					(*iter).player_2_.session_->handle_send(logic_server::GAME_STATE_NTF, start_ntf_packet);

					(*iter).match_log_->info("game start, room_key:{}, player_1_key:{}, player_2_key:{}",
						(*iter).room_key_,
						(*iter).player_1_.key_,
						(*iter).player_2_.key_);

					logic_server::packet_process_turn_req turn_req_packet;

					turn_req_packet.set_card_number(10);
					turn_req_packet.set_money(10);

					(*iter).player_1_.session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);

					turn_req_packet.set_card_number(12);
					(*iter).player_2_.session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);
				}
			}
			break;

			case ROOM_INFO::START:
			{
				if ((*iter).player_1_.submit_card_ && (*iter).player_2_.submit_card_)
				{
					(*iter).player_1_.submit_card_ = false;
					(*iter).player_2_.submit_card_ = false;

					(*iter).match_log_->info("turn play result, room_key:{}, player_1_key:{}, player_2_key:{}",
						(*iter).room_key_,
						(*iter).player_1_.key_,
						(*iter).player_2_.key_);

					logic_server::packet_process_turn_req turn_req_packet;

					turn_req_packet.set_card_number(5);
					turn_req_packet.set_money((*iter).player_1_.money_);

					(*iter).player_1_.session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);

					turn_req_packet.set_card_number(8);
					turn_req_packet.set_money((*iter).player_2_.money_);
					(*iter).player_2_.session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);
				}
			}
			break;

			case ROOM_INFO::END:
			{
			}
			break;
			}
		}

		LeaveCriticalSection(&critical_section_);
	}
}
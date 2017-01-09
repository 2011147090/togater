#include "logic_worker.h"

logic_worker::logic_worker() : logic_thread(NULL)
{
	InitializeCriticalSection(&critical_section);
}

logic_worker::~logic_worker() 
{
	DeleteCriticalSection(&critical_section);
}

bool logic_worker::init_singleton()
{
	if (logic_thread == NULL)
		logic_thread = new boost::thread(&logic_worker::process_queue, this);

	return true;
}

bool logic_worker::create_room(connected_session* session, int room_key, int player_key)
{
	EnterCriticalSection(&critical_section);

	if (!redis_manager::get_instance()->check_room(room_key))
	{
		LeaveCriticalSection(&critical_section);
		return false;
	}

	ROOM_INFO room_info;
	
	room_info.player_1_.key_ = player_key;
	room_info.player_1_.session = session;
	room_info.ready_player_num = 0;
	room_info.room_key_ = room_key;
	room_info.time = 0;

	room_list_.push_back(room_info);

	room_info.match_log->info("create_room, room_key:{}", room_key);

	LeaveCriticalSection(&critical_section);

	return true;
}

bool logic_worker::enter_room_player(connected_session* session, int room_key, int player_key)
{
	EnterCriticalSection(&critical_section);
	
	for (auto iter = room_list_.begin(); iter != room_list_.end(); iter++)
	{
		if ((*iter).room_key_ == room_key)
		{
			(*iter).ready_player_num++;
			(*iter).player_2_.key_ = player_key;
			(*iter).player_2_.session = session;

			(*iter).match_log->info("endter_room_player, room_key:{}, player_key:{}", player_key);

			LeaveCriticalSection(&critical_section);
			return true;
		}
	}

	if (create_room(session, room_key, player_key))
	{
		LeaveCriticalSection(&critical_section);
		return true;
	}

	LeaveCriticalSection(&critical_section);

	return false;
}

bool logic_worker::submit_card()
{
	EnterCriticalSection(&critical_section);

	/*for (auto iter = room_list_.begin(); iter != room_list_.end(); iter++)
	{
		if ((*iter).player_1_.key_ == player_key)
		{
			if (!(*iter).player_1_.submit_card)
			{
				(*iter).player_1_.submit_card_num = 0;
				(*iter).player_1_.submit_card = true;
			}

			(*iter).match_log->info("submit_card, key:{}, type:{}", player_key, type);

			LeaveCriticalSection(&critical_section);
			
			break;
		}
		else if ((*iter).player_2_.key_ == player_key)
		{
			if (!(*iter).player_2_.submit_card)
			{
				(*iter).player_2_.submit_card_num = 0;
				(*iter).player_2_.submit_card = true;
			}

			(*iter).match_log->info("submit_card, key:{}, type:{}", player_key, type);

			LeaveCriticalSection(&critical_section);

			break;
		}
	}*/

	LeaveCriticalSection(&critical_section);

	return false;
}

void logic_worker::process_queue()
{
	while (true)
	{

	}
}
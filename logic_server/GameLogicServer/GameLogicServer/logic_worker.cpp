#include "logic_worker.h"
#include "random_generator.h"

_PLAYER_INFO::_PLAYER_INFO()
{
	money_ = 0;
	total_money_ = 0;
	remain_money_ = 10;
	submit_card_ = false;
}

_ROOM_INFO::_ROOM_INFO()
{
	ready_player_num_ = 1;
	time_ = 0;
	turn_count_ = 0;
	state_ = GAME_STATE::READY;

	match_log_ = spd::daily_logger_st("logic_server", "match_log", 0, 0);
	spd::drop_all();
}

void _ROOM_INFO::generate_card_queue()
{
	while (!card_list_.empty())
		card_list_.pop();

	std::deque<int> d;
	
	for (int i = 0; i < 4; i++)
		for (int j = 1; j <= 10; j++)
			d.push_back(j);

	std::random_shuffle(d.begin(), d.end());
	card_list_ = std::queue<int>(d);
}

int _ROOM_INFO::get_card()
{
	if (card_list_.empty())
		generate_card_queue();

	int num = card_list_.front();
	card_list_.pop();

	return num;
}

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
	room_info.room_key_ = room_key;

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

bool comp(int const& a, int const& b) {
	if (a >= b) return true;

	return false;
}

logic_worker::HOLDEM_HANDS logic_worker::check_card_mix(int i, int j, int k)
{
	if (i == j)
		if (i == k)
			return HOLDEM_HANDS::TRIPLE;

	if (i == j || i == k || j == k)
		return HOLDEM_HANDS::PAIR;

	int iarray[3] = {i, j, k};
	std::sort(iarray, iarray + 2, comp);

	if (iarray[0] + 1 == iarray[1])
		if (iarray[1] + 1 == iarray[2])
			return HOLDEM_HANDS::STRAIGHT;

	return HOLDEM_HANDS::NONE;
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
					(*iter).state_ = ROOM_INFO::PLAYING;

					logic_server::packet_game_state_ntf start_ntf_packet;

					start_ntf_packet.set_state(1);

					(*iter).player_1_.session_->handle_send(logic_server::GAME_STATE_NTF, start_ntf_packet);
					(*iter).player_2_.session_->handle_send(logic_server::GAME_STATE_NTF, start_ntf_packet);

					(*iter).match_log_->info("game start, room_key:{}, player_1_key:{}, player_2_key:{}",
						(*iter).room_key_,
						(*iter).player_1_.key_,
						(*iter).player_2_.key_);

					logic_server::packet_process_turn_req turn_req_packet;
					
					(*iter).player_1_.opponent_card_num_ = (*iter).get_card();
					(*iter).player_2_.opponent_card_num_ = (*iter).get_card();
					
					(*iter).public_card_[0] = (*iter).get_card();
					(*iter).public_card_[1] = (*iter).get_card();
					turn_req_packet.set_public_card_number_1((*iter).public_card_[0]);
					turn_req_packet.set_public_card_number_2((*iter).public_card_[1]);

					turn_req_packet.set_card_number((*iter).player_1_.opponent_card_num_);
					turn_req_packet.set_money((*iter).player_1_.remain_money_);
					turn_req_packet.set_total_money(0);

					(*iter).player_1_.session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);

					turn_req_packet.set_card_number((*iter).player_2_.opponent_card_num_);
					turn_req_packet.set_money((*iter).player_2_.remain_money_);
					turn_req_packet.set_total_money(0);
					(*iter).player_2_.session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);
				}
			}
			break;

			case ROOM_INFO::PLAYING:
			{
				if ((*iter).player_1_.submit_card_ && (*iter).player_2_.submit_card_)
				{
					(*iter).player_1_.submit_card_ = false;
					(*iter).player_2_.submit_card_ = false;

					(*iter).match_log_->info("turn play result, room_key:{}, player_1_key:{}, player_2_key:{}",
						(*iter).room_key_,
						(*iter).player_1_.key_,
						(*iter).player_2_.key_);

					(*iter).player_1_.total_money_ += (*iter).player_1_.money_;
					(*iter).player_2_.total_money_ += (*iter).player_2_.money_;

					bool is_die = false;
					
					logic_server::packet_process_turn_req turn_req_packet;

					if ((*iter).player_1_.money_ == 0 || (*iter).player_2_.money_ == 0)
						is_die = true;

					if (is_die) {
						HOLDEM_HANDS player_1_result = check_card_mix(
							(*iter).player_2_.opponent_card_num_,
							(*iter).public_card_[0],
							(*iter).public_card_[1]
						);

						HOLDEM_HANDS player_2_result = check_card_mix(
							(*iter).player_1_.opponent_card_num_,
							(*iter).public_card_[0],
							(*iter).public_card_[1]
						);

						if (player_1_result > player_2_result)
						{
							(*iter).player_1_.remain_money_ += (*iter).player_2_.total_money_;
							(*iter).player_2_.remain_money_ -= (*iter).player_2_.total_money_;
						}
						else if (player_1_result < player_2_result)
						{
							(*iter).player_1_.remain_money_ -= (*iter).player_1_.total_money_;
							(*iter).player_2_.remain_money_ += (*iter).player_1_.total_money_;
						}
						else
						{
							if (player_1_result == HOLDEM_HANDS::NONE)
							{
								if ((*iter).player_1_.opponent_card_num_ > (*iter).player_2_.opponent_card_num_)
								{
									(*iter).player_1_.remain_money_ += (*iter).player_2_.total_money_;
									(*iter).player_2_.remain_money_ -= (*iter).player_2_.total_money_;
								}
								else if ((*iter).player_1_.opponent_card_num_ > (*iter).player_2_.opponent_card_num_)
								{
									(*iter).player_1_.remain_money_ -= (*iter).player_1_.total_money_;
									(*iter).player_2_.remain_money_ += (*iter).player_1_.total_money_;
								}
							}

						}

						(*iter).player_1_.opponent_card_num_ = (*iter).get_card();
						(*iter).player_2_.opponent_card_num_ = (*iter).get_card();

						(*iter).public_card_[0] = (*iter).get_card();
						(*iter).public_card_[1] = (*iter).get_card();
					}

					turn_req_packet.set_public_card_number_1((*iter).public_card_[0]);
					turn_req_packet.set_public_card_number_2((*iter).public_card_[1]);
						
					turn_req_packet.set_total_money((*iter).player_1_.total_money_);
					turn_req_packet.set_card_number((*iter).player_1_.opponent_card_num_);
					turn_req_packet.set_money((*iter).player_1_.remain_money_);

					(*iter).player_1_.session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);

					turn_req_packet.set_total_money((*iter).player_2_.total_money_);
					turn_req_packet.set_card_number((*iter).player_2_.opponent_card_num_);
					turn_req_packet.set_money((*iter).player_2_.remain_money_);

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
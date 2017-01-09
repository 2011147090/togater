#pragma once
#include "preHeaders.h"
#include "singleton.h"
#include "redis_manager.h"
#include "connected_session.h"

typedef struct _PLAYER_INFO {
	int key_;

	bool submit_card;
	int submit_card_num;

	connected_session* session;

	int money;

	_PLAYER_INFO()
	{
		money = 10;
		submit_card = false;
	}

} PLAYER_INFO;

typedef struct _ROOM_INFO {
	PLAYER_INFO player_1_;
	PLAYER_INFO player_2_;
	
	int room_key_;
	int time;

	int ready_player_num;

	std::shared_ptr<spd::logger> match_log;

	_ROOM_INFO()
	{
		match_log = spd::daily_logger_st("logic_server", "match_log", 0, 0);
	}

} ROOM_INFO;

class logic_worker : public singleton<logic_worker> {
public:
	virtual bool init_singleton();
	
	bool enter_room_player(connected_session* session, int room_key, int player_key);
	bool submit_card();
	void process_queue();

	logic_worker();
	~logic_worker();

private:
	std::vector<ROOM_INFO> room_list_;
	boost::thread* logic_thread;
	CRITICAL_SECTION critical_section;

	bool create_room(connected_session* session, int room_key, int player_key);
};
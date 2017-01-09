#pragma once
#include "preHeaders.h"
#include "singleton.h"
#include "redis_manager.h"
#include "connected_session.h"

typedef struct _PLAYER_INFO {
	int key_;

	bool submit_card_;
	int submit_card_num_;

	connected_session* session_;

	int total_money_;
	int money_;

	_PLAYER_INFO();

} PLAYER_INFO;

typedef struct _ROOM_INFO {
	PLAYER_INFO player_1_;
	PLAYER_INFO player_2_;
	
	int room_key_;
	int time_;
	int turn_count_;

	int ready_player_num_;

	enum GAME_STATE { READY, PLAYING, END };
	GAME_STATE state_;

	int public_card_[2];

	std::queue<int> card_list_;

	std::shared_ptr<spd::logger> match_log_;

	_ROOM_INFO();

	void generate_card_queue();
	int get_card();

} ROOM_INFO;

class logic_worker : public singleton<logic_worker> {
public:
	virtual bool init_singleton();
	
	bool enter_room_player(connected_session* session, int room_key, int player_key);
	bool process_turn(int player_key, int money);
	void process_queue();

	logic_worker();
	~logic_worker();

private:
	std::vector<ROOM_INFO> room_list_;
	boost::thread* logic_thread_;
	CRITICAL_SECTION critical_section_;

	enum HOLDEM_HANDS { NONE = 0, STRAIGHT = 1, PAIR = 2, TRIPLE = 3 };
	HOLDEM_HANDS check_card_mix(int i, int j, int k);

	bool create_room(connected_session* session, int room_key, int player_key);
};
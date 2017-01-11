#pragma once
#include "preHeaders.h"
#include "singleton.h"
#include "redis_manager.h"
#include "connected_session.h"
#include "critical_section.h"

typedef struct _PLAYER_INFO {
    int key_;
    std::string id_;

    bool submit_card_;
    int opponent_card_num_;

    connected_session* session_;

    int sum_money_;
    int remain_money_;
    int money_;

    _PLAYER_INFO();

} PLAYER_INFO;

typedef struct _ROOM_INFO {
    PLAYER_INFO player_[2];

    PLAYER_INFO* turn_player_;
    
    std::string room_key_;
    int time_;
    int turn_count_;

    int ready_player_num_;

    enum GAME_STATE { READY, PLAYING, END };
    GAME_STATE state_;

    int public_card_[2];

    std::queue<int> card_list_;

    _ROOM_INFO();

    void generate_card_queue();
    int get_card();

} ROOM_INFO;

class logic_worker : public singleton<logic_worker>, public multi_thread_sync<logic_worker> {
public:
    virtual bool init_singleton();
    
    bool enter_room_player(connected_session* session, std::string room_key, int player_key);
    bool process_turn(int player_key, int money);
    void process_queue();

    logic_worker();
    ~logic_worker();

private:
    std::vector<ROOM_INFO> room_list_;
    boost::thread* logic_thread_;

    enum HOLDEM_HANDS { NONE = 0, STRAIGHT = 1, PAIR = 2, TRIPLE = 3 };
    HOLDEM_HANDS check_card_mix(int i, int j, int k);
    int get_top_card(int i, int k, int j);

    bool create_room(connected_session* session, std::string room_key, int player_key);
};
#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "cocos2d.h"
#include "player.h"
#include "main_scene.h"
#include "singleton.h"
#include "holdem_card.h"
#include "ui\UIButton.h"
#include "ui\UITextField.h"

class game_manager : public singleton<game_manager>
{
public:
    virtual bool init_singleton();
    virtual bool release_singleton();

    game_manager();

    int total_money_;
    int batting_money_;
    bool wait_turn;

    player* user_;
    player* opponent_;
    
    holdem_card* public_card_;
    holdem_card* opponent_card_;

    cocos2d::Scheduler* scheduler_;
    
    cocos2d::ui::Button* bet_button_;
    cocos2d::ui::TextField* text_field_;

    main_scene* scene_;
    
    bool hide_card_;

    void new_turn(int public_card_1, int public_card_2, int opponent_card, int remain_money, int my_money, int opponent_money);
    void opponent_turn_end(int my_money, int opponent_money);
    void betting(int player_key);

    void check_public_card();

    void set_player_key(std::string key);
    std::string get_player_key();

    void start_game();

private:
    std::string player_key_;
};


#define game_mgr game_manager::get_instance()
#endif // __GAME_MANAGER_H__

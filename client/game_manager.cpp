#include "game_manager.h"
#include "network_manager.h"
#include "main_scene.h"
#include "logic_session.h"
#include "ui\UIText.h"

bool game_manager::init_singleton()
{
    public_card_ = nullptr;
    opponent_card_ = nullptr;

    return true;
}

bool game_manager::release_singleton()
{
    if (user_ != nullptr)
    {
        delete user_;
        user_ = nullptr;
    }

    if (opponent_ != nullptr)
    {
        delete opponent_;
        opponent_ = nullptr;
    }

    return true;
}

game_manager::game_manager()
{
    
}

void game_manager::new_turn(int public_card_1, int public_card_2, int opponent_card, int remain_money, int my_money, int opponent_money)
{
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    cocos2d::Vec2 origin = cocos2d::Director::getInstance()->getVisibleOrigin();
    cocos2d::Vec2 middle_pos(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    if (remain_money > user_->get_coin_size() + user_->get_bet_coin_size())
    {
        opponent_->set_bet_coin(
            remain_money - opponent_->get_bet_coin_size() - (user_->get_bet_coin_size() + user_->get_coin_size())
        );
        user_->reset_coin(opponent_);
    }
    else if (my_money == 0 && opponent_money == 0)
        opponent_->reset_coin(user_);
   
    user_->set_lock_bet_size(my_money + 1);
    opponent_->set_lock_bet_size(opponent_money + 1);

    user_->set_bet_coin(1);
    opponent_->set_bet_coin(1);
   
    if (public_card_ != nullptr)
    {
        public_card_[0].release();
        public_card_[1].release();

        delete[] public_card_;
    }

    if (opponent_card_ != nullptr)
    {
        opponent_card_->release();

        delete opponent_card_;
    }

    public_card_ = new holdem_card[2] {
        holdem_card(2, public_card_1, true, "Public Card"),
        holdem_card(2, public_card_2, false, "Public Card")
    };

    public_card_[0].move_action(middle_pos + cocos2d::Vec2(-180, 20), 2);
    public_card_[1].move_action(middle_pos + cocos2d::Vec2(-80, 20), 2);

    opponent_card_ = new holdem_card(2, opponent_card, false, "Opponent Card");
    opponent_card_->move_action(cocos2d::Vec2(270, visibleSize.height - 100), 2);
}

void game_manager::betting(int player_key)
{ 
    if (user_->get_bet_coin_size() >= opponent_->get_bet_coin_size()
        || user_->get_bet_coin_size() - user_->get_lock_bet_size() == 0 || user_->get_coin_size() == 0)
    {
        network_logic->send_packet_process_turn_ans(user_->get_bet_coin_size() - user_->get_lock_bet_size());

        user_->set_lock_bet_size(user_->get_bet_coin_size());

        bet_button_->setEnabled(false);
    }
}

void game_manager::opponent_turn_end(int my_money, int opponent_money)
{
    int size = opponent_->get_bet_coin_size();
    
    for (int i = 0; i < opponent_money - size; i++)
       opponent_->bet_coin();

    user_->set_lock_bet_size(my_money);

    bet_button_->setEnabled(true);
}

void game_manager::check_public_card()
{
    public_card_[0].show();
}

void game_manager::start_game()
{
    auto scene = main_scene::createScene();

    ((main_scene*)scene)->setup_scene();
    
    cocos2d::Director::getInstance()->pushScene(cocos2d::TransitionFade::create(1, scene));
}

void game_manager::add_lobby_chat(std::string id, std::string str)
{
    std::string message = id;
    message += " : ";
    message += str;
    
    auto label = cocos2d::ui::Text::create(message, "fonts/D2Coding.ttf", 15);
    label->setTextColor(cocos2d::Color4B::BLACK);
    label->setTouchEnabled(true);
    this->lobby_chat_list_->pushBackCustomItem(label);
}
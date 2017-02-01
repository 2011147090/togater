#include "main_scene.h"
#include "SimpleAudioEngine.h"
#include "ui\UILayout.h"
#include "ui\UITextField.h"
#include "ui\UIButton.h"
#include "holdem_card.h"
#include "game_manager.h"
#include "network_manager.h"
#include "chat_session.h"
#include "logic_session.h"
#include "channel_session.h"

USING_NS_CC;

ui::TextField* room_chat_field;

Scene* main_scene::createScene()
{
    auto scene_ = Scene::create();

    auto layer = main_scene::create();

    scene_->addChild(layer);

    return scene_;
}

bool main_scene::init()
{
    if (!Layer::init())
        return false;

    return true;
}

void main_scene::setup_scene()
{
#pragma region Init_UI
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    Vec2 middle_pos(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    auto back = ui::Button::create("button2_normal.png", "button2_pressed.png");

    back->setTitleText("Give Up");
    back->setTitleFontName("fonts/D2Coding.ttf");
    back->setTitleFontSize(20);
    back->setTitleColor(Color3B::BLACK);
    back->setScale(1.0f, 1.0);
    back->setAnchorPoint(Vec2(0.5, 0.5));
    back->setPosition(Vec2(visibleSize.width - 100, 100));

    auto background = Sprite::create("background.jpg");
    background->setScale(0.8f);
    background->setPosition(middle_pos);
    this->addChild(background, 0);

    auto label_info_1 = cocos2d::Label::createWithTTF("Public Card", "fonts/arial.ttf", 10);
    label_info_1->setColor(cocos2d::Color3B::WHITE);
    label_info_1->setPosition(middle_pos + cocos2d::Vec2(-180, -35));
    this->addChild(label_info_1, 1);

    auto label_info_2 = cocos2d::Label::createWithTTF("Public Card", "fonts/arial.ttf", 10);
    label_info_2->setColor(cocos2d::Color3B::WHITE);
    label_info_2->setPosition(middle_pos + cocos2d::Vec2(-80, -35));
    this->addChild(label_info_2, 1);

    auto label_info_3 = cocos2d::Label::createWithTTF("Opponent Card", "fonts/arial.ttf", 10);
    label_info_3->setColor(cocos2d::Color3B::WHITE);
    label_info_3->setPosition(middle_pos + cocos2d::Vec2(-130, 85));
    this->addChild(label_info_3, 1);

    auto bet_coin_user = cocos2d::Label::createWithTTF("Bet : 0", "fonts/arial.ttf", 15);
    bet_coin_user->setColor(cocos2d::Color3B::ORANGE);
    bet_coin_user->setPosition(cocos2d::Vec2(visibleSize.width - 100, 150));
    this->addChild(bet_coin_user, 1);

    auto opponent_info = cocos2d::Label::createWithTTF("ID : Temp\nWin : 0, Defeat : 0\nRating : 1200\nBet : 0", "fonts/arial.ttf", 15);
    opponent_info->setColor(cocos2d::Color3B::RED);
    opponent_info->setPosition(cocos2d::Vec2(visibleSize.width - 100, visibleSize.height - 100));
    this->addChild(opponent_info, 1);
    
    auto card_pack = Sprite::create("card_pack.png");
    card_pack->setScale(1.0f);
    card_pack->setPosition(middle_pos - Vec2(150, 110));
    this->addChild(card_pack, 1);

    auto label_card_pack = cocos2d::Label::createWithTTF("Server\nCamp", "fonts/arial.ttf", 15, Size::ZERO, cocos2d::TextHAlignment::CENTER);
    label_card_pack->setColor(cocos2d::Color3B::BLACK);
    label_card_pack->setPosition(middle_pos - Vec2(155, 110));
    this->addChild(label_card_pack, 2);

    auto chat_background = Sprite::create("in_game_chat_frame.png");
    chat_background->setAnchorPoint(Vec2(0, 0));
    chat_background->setPosition(Vec2(0, 0));
    this->addChild(chat_background, 4);

    room_chat_field = ui::TextField::create("Input Chat Here", "fonts/D2Coding.ttf", 15);
    room_chat_field->setMaxLength(10);
    room_chat_field->setColor(cocos2d::Color3B::BLACK);
    room_chat_field->setMaxLength(true);
    room_chat_field->setAnchorPoint(Vec2(0, 0));
    room_chat_field->setPosition(Vec2(9, 5));

    this->addChild(room_chat_field, 5);

    auto chat_list = ui::ListView::create();
    chat_list->setDirection(ui::ListView::Direction::VERTICAL);
    chat_list->setClippingEnabled(true);
    chat_list->setTouchEnabled(true);
    chat_list->setContentSize(Size(210, 260));
    chat_list->setAnchorPoint(Vec2(0.5, 1));
    chat_list->setBounceEnabled(false);
    chat_list->setScrollBarEnabled(true);
    chat_list->setScrollBarPositionFromCorner(Vec2(0, 0));
    chat_list->setItemsMargin(2.0f);
    chat_list->setPosition(Vec2(30, visibleSize.height - 50));
    this->addChild(chat_list, 5);
    
    auto chat_button = ui::Button::create("button3_normal.png", "button3_pressed.png");

    chat_button->setTitleText("");
    chat_button->setTitleFontName("fonts/D2Coding.ttf");
    chat_button->setTitleFontSize(20);
    chat_button->setTitleColor(Color3B::BLACK);
    chat_button->setScale(0.6f, 0.6f);
    chat_button->setAnchorPoint(Vec2(0.5, 0.5));
    chat_button->setPosition(Vec2(140, 13));

    auto bet_button = ui::Button::create("bet_button_normal.png", "bet_button_pressed.png", "bet_button_disable.png");
    bet_button->setTitleText("BET");
    bet_button->setTitleFontSize(20);
    bet_button->setEnabled(false);
    bet_button->setPosition(Vec2(visibleSize.width - 100, visibleSize.height / 2));

#pragma endregion

#pragma region Listener Event Settings
    auto keylistener = EventListenerKeyboard::create();
    keylistener->onKeyReleased = CC_CALLBACK_2(main_scene::on_key_released, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keylistener, this);

    auto listener = EventListenerTouchOneByOne::create();

    listener->setSwallowTouches(true);

    listener->onTouchBegan = CC_CALLBACK_2(main_scene::on_touch_begin, this);
    listener->onTouchMoved = CC_CALLBACK_2(main_scene::on_touch_moved, this);
    listener->onTouchCancelled = CC_CALLBACK_2(main_scene::on_touch_cancelled, this);
    listener->onTouchEnded = CC_CALLBACK_2(main_scene::on_touch_ended, this);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    bet_button->addTouchEventListener([&](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            game_mgr->betting();
            break;
        }
    });

    this->addChild(bet_button);

    chat_button->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            network_chat->send_packet_chat_room(
                network_mgr->get_player_id(),
                room_chat_field->getString()
            );

            room_chat_field->setText("");
            break;
        }
    });

    this->addChild(chat_button, 6);

    back->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            network_logic->send_packet_disconnect_room_ntf();
            break;
        }
    });

    this->addChild(back, 1);
#pragma endregion

#pragma region GameManager Member Settings

    game_mgr->scene_ = this;
    game_mgr->scheduler_[game_manager::ROOM] = this->getScheduler();

    game_mgr->room_chat_list_ = chat_list;
    game_mgr->set_scene_status(game_manager::SCENE_TYPE::ROOM);
    
    game_mgr->user_ = new player();
    game_mgr->opponent_ = new player();

    game_mgr->user_->init(true);
    game_mgr->opponent_->init(false);

    game_mgr->bet_button_ = bet_button;

    game_mgr->user_bet_text_ = bet_coin_user;
    game_mgr->opponent_info_text_ = opponent_info;
    
    opponent_info->setString(game_mgr->opponent_info_);

#pragma endregion 
}

void main_scene::end()
{
    network_logic->destroy();
    network_lobby->create();

    game_mgr->release_singleton();

    if (!network_lobby->is_run())
    {
        this->getScheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                channel_session::connect,
                network_lobby,
                CHANNEL_SERVER_IP, CHANNEL_SEFVER_PORT
            )
        );

        this->getScheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                channel_session::send_packet_join_req,
                network_lobby,
                network_mgr->get_player_key(),
                network_mgr->get_player_id()
            )
        );
    }

    this->getScheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            chat_session::send_packet_leave_match_ntf,
            network_chat
        )
    );
  
    Director::getInstance()->popScene();
    Director::getInstance()->popScene();

    game_mgr->set_scene_status(game_manager::SCENE_TYPE::LOBBY);
}

void main_scene::on_key_released(EventKeyboard::KeyCode keyCode, Event* event)
{
    switch (keyCode)
    {
    case EventKeyboard::KeyCode::KEY_ENTER:
        
        break;
    }
}

void main_scene::on_touch_ended(cocos2d::Touch* touch, Event *unused_event)
{
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    cocos2d::Vec2 origin = cocos2d::Director::getInstance()->getVisibleOrigin();

    cocos2d::Vec2 middle_pos(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
    middle_pos += cocos2d::Vec2(100 + rand() % 100, rand() % 100);

    bool move_coin = false;

    if (touch->getLocation().x >= 210 && touch->getLocation().x <= 220 + game_mgr->user_->get_coin_size() * 10)
    {
        if (touch->getLocation().y >= 0 && touch->getLocation().y <= 60)
        {
            game_mgr->user_->bet_coin();

            move_coin = true;
        }
    }

    if (touch->getLocation().x >= middle_pos.x - 100 && touch->getLocation().x <= middle_pos.x + 100)
    {
        if (touch->getLocation().y >= middle_pos.y - 100 && touch->getLocation().y <= middle_pos.y + 100)
        {
            game_mgr->user_->restore_coin();

            move_coin = true;
        }
    }

    if (move_coin)
    {
        int bet_size = game_mgr->user_->get_bet_coin_size();
        char temp[5] = "";
        itoa(bet_size, temp, 10);

        std::string bet_text = "Bet : ";
        bet_text += temp;

        game_mgr->user_bet_text_->setString(bet_text);
    }
}

void main_scene::menu_close_callback(Ref* sender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

bool main_scene::on_touch_begin(cocos2d::Touch *touch, cocos2d::Event *unused_event) { return true; }
void main_scene::on_touch_moved(cocos2d::Touch *touch, cocos2d::Event *unused_event) {}
void main_scene::on_touch_cancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event) {}
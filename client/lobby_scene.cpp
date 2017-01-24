#include "lobby_scene.h"
#include <ui\UIButton.h>
#include <ui\UITextField.h>
#include <ui\UIText.h>
#include <array>
#include "main_scene.h"
#include "loading_scene.h"
#include "game_manager.h"
#include "network_manager.h"
#include "chat_session.h"

using namespace cocos2d;

ui::TextField* chat_field;
ui::ListView* chat_list;

Scene* lobby_scene::createScene()
{
    auto scene = Scene::create();

    auto layer = lobby_scene::create();

    scene->addChild(layer);

    return scene;
}

bool lobby_scene::init()
{
    if (!LayerColor::initWithColor(Color4B(0, 255, 0, 255)))
        return false;

    auto winSize = CCDirector::sharedDirector()->getWinSizeInPixels();
    auto visibleSize = Director::getInstance()->getVisibleSize();

    auto background = Sprite::create("lobby_background.png");
    background->setAnchorPoint(Vec2(0, 0));
    background->setScale(1.28f);
    this->addChild(background, 0);


    auto back = ui::Button::create("button2_normal.png", "button2_pressed.png");

    back->setTitleText("Back");
    back->setTitleFontName("fonts/D2Coding.ttf");
    back->setTitleFontSize(20);
    back->setTitleColor(Color3B::BLACK);
    back->setScale(1.62f, 1.9f);
    back->setAnchorPoint(Vec2(0, 1));
    back->setPosition(Vec2(25, visibleSize.height - 20));
    back->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            Director::getInstance()->popScene();
            break;
        }
    });

    this->addChild(back, 1);


    auto rank_match = ui::Button::create("button2_normal.png", "button2_pressed.png");

    rank_match->setTitleText("Rank Match");
    rank_match->setTitleFontName("fonts/D2Coding.ttf");
    rank_match->setTitleFontSize(20);
    rank_match->setTitleColor(Color3B::BLACK);
    rank_match->setScale(1.62f, 1.9f);
    rank_match->setAnchorPoint(Vec2(0, 1));
    rank_match->setPosition(Vec2(280, visibleSize.height - 20));
    rank_match->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            match_game();
            break;
        }
    });

    this->addChild(rank_match, 1);


    auto add_friend = ui::Button::create("button_normal.png", "button_pressed.png");

    add_friend->setTitleText("Add");
    add_friend->setTitleFontName("fonts/D2Coding.ttf");
    add_friend->setTitleFontSize(20);
    add_friend->setTitleColor(Color3B::BLACK);
    add_friend->setScale(0.74f, 0.8f);
    add_friend->setAnchorPoint(Vec2(1, 1));
    add_friend->setPosition(Vec2(visibleSize.width - 146, 165));
    add_friend->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            break;
        }
    });

    this->addChild(add_friend, 1);


    auto remove_friend = ui::Button::create("button_normal.png", "button_pressed.png");

    remove_friend->setTitleText("Remove");
    remove_friend->setTitleFontName("fonts/D2Coding.ttf");
    remove_friend->setTitleFontSize(20);
    remove_friend->setTitleColor(Color3B::BLACK);
    remove_friend->setScale(0.74f, 0.8f);
    remove_friend->setAnchorPoint(Vec2(1, 1));
    remove_friend->setPosition(Vec2(visibleSize.width - 33, 165));
    remove_friend->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            break;
        }
    });

    this->addChild(remove_friend, 1);


    auto friend_match = ui::Button::create("button_normal.png", "button_pressed.png");

    friend_match->setTitleText("Friend Match");
    friend_match->setTitleFontName("fonts/D2Coding.ttf");
    friend_match->setTitleFontSize(20);
    friend_match->setTitleColor(Color3B::BLACK);
    friend_match->setScale(1.5f, 1.5f);
    friend_match->setAnchorPoint(Vec2(1, 1));
    friend_match->setPosition(Vec2(visibleSize.width - 33, 85));
    friend_match->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            break;
        }
    });

    this->addChild(friend_match, 1);


    auto friend_list = ui::ListView::create();
    friend_list->setDirection(ui::ListView::Direction::VERTICAL);
    friend_list->setClippingEnabled(true);
    friend_list->setTouchEnabled(true);
    friend_list->setContentSize(Size(210, 240));
    friend_list->setAnchorPoint(Vec2(0.5, 1));
    friend_list->setBounceEnabled(false);
    friend_list->setScrollBarEnabled(true);
    friend_list->setScrollBarPositionFromCorner(Vec2(1, 0.5f));
    friend_list->setItemsMargin(2.0f);

    friend_list->setPosition(Vec2(visibleSize.width/2 + 255, visibleSize.height/2 + 170));
    
    std::array<std::string, 12> names = { 
        "Arika", "Sejong", "LeeTaeHyoung", "Junbum", "Garam", 
        "Gyeong Hyun", "Mike", "Nami", "respect", "WhoYouAre", "HelloWorld",
        "SoSo"
    };

    for(auto iter = names.begin(); iter != names.end(); iter++)
    {
        auto label = ui::Text::create(*iter, "fonts/D2Coding.ttf", 20);
        label->setTextColor(Color4B::BLACK);
        label->setTouchEnabled(true);
        friend_list->pushBackCustomItem(label);
    }

    friend_list->addEventListener([this](Ref* sender, ui::ListView::EventType type)
    {

        if (type == ui::ListView::EventType::ON_SELECTED_ITEM_END) {

        }

    });

    this->addChild(friend_list, 2);


    auto friend_search_field = ui::TextField::create("Input Friend ID", "fonts/D2Coding.ttf", 20);
    friend_search_field->setMaxLength(10);
    friend_search_field->setMaxLengthEnabled(true);
    friend_search_field->setColor(cocos2d::Color3B::BLACK);
    friend_search_field->setPosition(Vec2(visibleSize.width / 2 + 255, visibleSize.height / 2 - 132));
    this->addChild(friend_search_field, 1);


    chat_list = ui::ListView::create();
    chat_list->setDirection(ui::ListView::Direction::VERTICAL);
    chat_list->setClippingEnabled(true);
    chat_list->setTouchEnabled(true);
    chat_list->setContentSize(Size(210, 260));
    chat_list->setAnchorPoint(Vec2(0.5, 1));
    chat_list->setBounceEnabled(false);
    chat_list->setScrollBarEnabled(true);
    chat_list->setScrollBarPositionFromCorner(Vec2(1, 0.5f));
    chat_list->setItemsMargin(2.0f);

    chat_list->setPosition(Vec2(145, visibleSize.height - 160));

    std::array<std::string, 1> chats = {
        "--Welcom Our Game--"
    };

    for (auto iter = chats.begin(); iter != chats.end(); iter++)
    {
        auto label = ui::Text::create(*iter, "fonts/D2Coding.ttf", 15);
        label->setTextColor(Color4B::BLACK);
        label->setTouchEnabled(true);
        chat_list->pushBackCustomItem(label);
    }
        
    chat_list->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(lobby_scene::chat_event_listener, this));
    this->addChild(chat_list, 2);

    game_mgr->scheduler_ = this->getScheduler();
    game_mgr->lobby_chat_list_ = chat_list;
    
    this->getScheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            chat_session::connect,
            network_chat,
            CHAT_SERVER_IP, "8700"
        )
    );

    this->getScheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            chat_session::send_packet_verify_req,
            network_chat,
            network_mgr->get_player_key(),
            network_mgr->get_player_id()
        )
    );
    
    auto chat_button = ui::Button::create("button3_normal.png", "button3_pressed.png");

    chat_button->setTitleText("");
    chat_button->setTitleFontName("fonts/D2Coding.ttf");
    chat_button->setTitleFontSize(20);
    chat_button->setTitleColor(Color3B::BLACK);
    chat_button->setScale(1.0f, 0.6f);
    chat_button->setAnchorPoint(Vec2(1, 1));
    chat_button->setPosition(Vec2(255, 46));
    chat_button->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            network_chat->send_packet_chat_normal(
                network_mgr->get_player_id(),
                chat_field->getString()
            );

            chat_field->setText("");
            break;
        }
    });

    this->addChild(chat_button, 2);


    chat_field = ui::TextField::create("Chat Message", "fonts/D2Coding.ttf", 15);
    chat_field->setMaxLength(20);
    chat_field->setMaxLengthEnabled(true);
    chat_field->setTextHorizontalAlignment(cocos2d::TextHAlignment::LEFT);
    chat_field->setAnchorPoint(Vec2(0, 0.5));
    chat_field->setColor(cocos2d::Color3B::BLACK);
    chat_field->setPosition(Vec2(40, 33));
    
    this->addChild(chat_field, 1);


    auto keyboard_listener = EventListenerKeyboard::create();
    keyboard_listener->onKeyPressed = CC_CALLBACK_2(lobby_scene::onKeyPressed, this);
    keyboard_listener->onKeyReleased = CC_CALLBACK_2(lobby_scene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboard_listener, this);
    
    return true;
}

void lobby_scene::match_game()
{
    auto scene = loading_scene::createScene();
    Director::getInstance()->pushScene(TransitionFade::create(2, scene));
}

void lobby_scene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
    if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ENTER)
    {
        if (chat_field->isFocused())
        {
            network_chat->send_packet_chat_normal(
                network_mgr->get_player_id(),
                chat_field->getString()
            );

            chat_field->setText("");
        }
    }
    /*
    else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_BACKSPACE)
    {
        if (input)
        {
            std::string str = chat_field->getString();

            if (str.length() <= 0)
                return;

            str.pop_back();
            chat_field->setText(str);
        }
    }*/
}
void lobby_scene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
    /*if (input == true)
    {
        if ((int)keyCode >= 60 && (int)keyCode <= 153)
            chat_field->setText(chat_field->getString() + (char)((int)keyCode - 27) + "\0");
        else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
            chat_field->setText(chat_field->getString() + " ");
    }*/
}

void lobby_scene::chat_event_listener(Ref* sender, cocos2d::ui::ListView::EventType type)
{
    /*if (type == ui::ListView::EventType::ON_SELECTED_ITEM_END) {
        auto label = ui::Text::create("ASD", "fonts/D2Coding.ttf", 15);
        label->setTextColor(Color4B::BLACK);
        label->setTouchEnabled(true);
        chat_list->pushBackCustomItem(label);
    }*/
}
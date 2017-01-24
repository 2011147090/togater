#include "main_scene.h"
#include "SimpleAudioEngine.h"
#include "ui\UILayout.h"
#include "ui\UITextField.h"
#include "ui\UIButton.h"
#include "holdem_card.h"
#include "game_manager.h"
#include "network_manager.h"
#include "logic_session.h"

USING_NS_CC;

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
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    Vec2 middle_pos(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    //auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
    //    CC_CALLBACK_1(main_scene::menu_close_callback, this));

    //closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width / 2,
    //    origin.y + closeItem->getContentSize().height / 2));

    // create menu, it's an autorelease object
    //auto menu = Menu::create(closeItem, nullptr);
    //menu->setPosition(Vec2::ZERO);
    //this->addChild(menu, 1);

    auto back = ui::Button::create("button2_normal.png", "button2_pressed.png");

    back->setTitleText("Give Up");
    back->setTitleFontName("fonts/D2Coding.ttf");
    back->setTitleFontSize(10);
    back->setTitleColor(Color3B::BLACK);
    back->setScale(1.0f, 1.0);
    back->setAnchorPoint(Vec2(0, 1));
    back->setPosition(Vec2(visibleSize.width - 100, 10));
    back->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            network_logic->send_packet_disconnect_room_ntf();
            break;
        }
    });


    this->addChild(back, 1);


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

    auto chat_field = ui::TextField::create("Input Chat Here", "fonts/arial.ttf", 15);
    chat_field->setMaxLength(10);
    chat_field->setColor(cocos2d::Color3B::BLACK);
    chat_field->setMaxLength(true);
    chat_field->setAnchorPoint(Vec2(0, 0));
    chat_field->setPosition(Vec2(9, 5));

    game_mgr->text_field_ = chat_field;
    
    this->addChild(chat_field, 5);

    auto keylistener = EventListenerKeyboard::create();
    keylistener->onKeyReleased = CC_CALLBACK_2(main_scene::on_key_released, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keylistener, this);

    game_mgr->scene_ = this;
    
    game_mgr->user_ = new player();
    game_mgr->opponent_ = new player();

    game_mgr->user_->init(true);
    game_mgr->opponent_->init(false);

    auto bet_button = ui::Button::create("bet_button_normal.png", "bet_button_pressed.png", "bet_button_disable.png");

    game_mgr->bet_button_ = bet_button;

    bet_button->setTitleText("BET");
    bet_button->setTitleFontSize(20);
    bet_button->setEnabled(false);
    bet_button
        ->setPosition(Vec2(visibleSize.width - 100, visibleSize.height / 2));
    bet_button->addTouchEventListener([&](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            game_mgr->betting(atoi(game_mgr->text_field_->getString().c_str()));
            break;
        }
    });

    this->addChild(bet_button);


    auto listener = EventListenerTouchOneByOne::create();

    listener->setSwallowTouches(true);

    listener->onTouchBegan = CC_CALLBACK_2(main_scene::on_touch_begin, this);
    listener->onTouchMoved = CC_CALLBACK_2(main_scene::on_touch_moved, this);
    listener->onTouchCancelled = CC_CALLBACK_2(main_scene::on_touch_cancelled, this);
    listener->onTouchEnded = CC_CALLBACK_2(main_scene::on_touch_ended, this);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void main_scene::end()
{
    network_logic->destroy();
    game_mgr->release_singleton();
  
    Director::getInstance()->popScene();
    Director::getInstance()->popScene();
}

void main_scene::on_key_released(EventKeyboard::KeyCode keyCode, Event* event)
{
    switch (keyCode)
    {
    case EventKeyboard::KeyCode::KEY_ENTER:
        
        break;
    }
}

bool main_scene::on_touch_begin(cocos2d::Touch *touch, cocos2d::Event *unused_event)
{
    return true;
}

void main_scene::on_touch_moved(cocos2d::Touch *touch, cocos2d::Event *unused_event)
{
}

void main_scene::on_touch_cancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event)
{
}


void main_scene::on_touch_ended(cocos2d::Touch* touch, Event *unused_event)
{
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    cocos2d::Vec2 origin = cocos2d::Director::getInstance()->getVisibleOrigin();

    cocos2d::Vec2 middle_pos(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
    middle_pos += cocos2d::Vec2(100 + rand() % 100, rand() % 100);

    if (touch->getLocation().x >= 210 && touch->getLocation().x <= 220 + game_mgr->user_->get_coin_size() * 10)
        if (touch->getLocation().y >= 0 && touch->getLocation().y <= 60)
            game_mgr->user_->bet_coin();

    if (touch->getLocation().x >= middle_pos.x - 100 && touch->getLocation().x <= middle_pos.x + 100)
        if (touch->getLocation().y >= middle_pos.y - 100 && touch->getLocation().y <= middle_pos.y + 100)
            game_mgr->user_->restore_coin();
}

void main_scene::menu_close_callback(Ref* sender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

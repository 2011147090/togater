#include "lobby_scene.h"
#include <ui\UIButton.h>
#include <ui\UITextField.h>
#include <ui\UIText.h>
#include <array>
#include "loading_scene.h"
#include "network_manager.h"
#include "game_manager.h"
#include "logic_session.h"

using namespace cocos2d;

Sprite* progress_bar;

Scene* loading_scene::createScene()
{
    auto scene = Scene::create();

    auto layer = loading_scene::create();

    scene->addChild(layer);

    return scene;
}

bool loading_scene::init()
{
    if (!LayerColor::initWithColor(Color4B(0, 255, 0, 255)))
        return false;

    auto winSize = CCDirector::sharedDirector()->getWinSizeInPixels();
    auto visibleSize = Director::getInstance()->getVisibleSize();

    auto background = Sprite::create("background.jpg");
    background->setAnchorPoint(Vec2(0, 0));
    background->setScale(1.28f);
    this->addChild(background, 0);
    
    auto loading_title = ui::Text::create("Searching For Enemy", "fonts/D2Coding.ttf", 25);
    loading_title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 80));
    loading_title->setTextColor(Color4B::WHITE);
    this->addChild(loading_title, 1);

    progress_bar = Sprite::create("progress_bar.png");
    progress_bar->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    progress_bar->setScale(0.5f);
    this->addChild(progress_bar, 1);

    scheduleUpdate();

    return true;
}

void loading_scene::update(float delta)
{
    static float depth = 0;
    depth += delta * 200;

    if (delta > 300)
        delta = 0;

    progress_bar->setRotation(depth);

    static float timer = 0;

    if (timer == 0)
    {
        this->getScheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                logic_session::connect, 
                network_logic,
                LOGIC_SERVER_IP, "8600"
            )
        );
    }

    if (timer < 3)
        timer += delta;

    if (timer > 3)
    {
        timer = 3;

        if (network_logic->is_run())
        {
            network_logic->send_packet_enter_req(
                "temp",
                network_mgr->get_player_key()
            );
        }
    }
}
#include "network_manager.h"
#include "logic_session.h"
#include "chat_session.h"

network_manager::network_manager()
{
    session[0] = nullptr;
    session[1] = nullptr;
}

network_manager::~network_manager() {}

bool network_manager::init_singleton()
{
    if (session[0] == nullptr)
    {
        session[0] = new logic_session();
        session[0]->create();
    }

    if (session[1] == nullptr)
    {
        session[1] = new chat_session();
        session[1]->create();
    }

    if (!session[0]->create())
        return false;

    if (!session[1]->create())
        return false;

    return true;
}

bool network_manager::release_singleton()
{
    if (session[0] != nullptr)
        if (!session[0]->destroy())
            return false;

    if (session[1] != nullptr)
        if (!session[1]->destroy())
            return false;

    delete[] session;

    session[0] = nullptr;
    session[1] = nullptr;
        
    return true;
}

network_session* network_manager::get_session(SESSION_TYPE session_type)
{
    switch (session_type) {
    case SESSION_TYPE::LOGIC_SESSION:
        return session[0];
        break;

    case SESSION_TYPE::CHAT_SESSION:
        return session[1];
        break;
    }

    return nullptr;
}

std::string network_manager::get_player_key()
{
    return player_key_;
}

std::string network_manager::get_player_id()
{
    return player_id_;
}

std::string network_manager::get_room_key()
{
    return room_key_;
}

void network_manager::set_player_key(std::string key)
{
    player_key_ = key;
}

void network_manager::set_player_id(std::string id)
{
    player_id_ = id;
}

void network_manager::set_room_key(std::string key)
{
    room_key_ = key;
}
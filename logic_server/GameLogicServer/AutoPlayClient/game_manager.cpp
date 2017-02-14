#include "game_manager.h"
#include "network_manager.h"
#include "random_generator.h"
#include "logger.h"
#include "configurator.h"

bool game_manager::init_singleton()
{
    send_friend_match_ = false;
    accept_friend_match_ = false;
    friend_match_id_ = "";
    end_game_ = false;
    state_ = READY;

    return true;
}

bool game_manager::release_singleton()
{
    return true;
}

void game_manager::play_game(std::string id, std::string password)
{
    char temp[128] = "";

    if (!network_mgr->try_login(id, password))
    {
        sprintf(temp, "login faield : id(%s), pwd(%s)", id, password);

        logger::print(temp);

        return;
    }

    sprintf(temp, "login success : id(%s), pwd(%s)", id, password);
    logger::print(temp);
    
#pragma region Lobby_Scene
    network_chat->create();
    std::string chat_server_ip;
    std::string chat_server_port;
    configurator::get_value("chat_server_ip", chat_server_ip);
    configurator::get_value("chat_server_port", chat_server_port);
    network_chat->connect(chat_server_ip, chat_server_port);
    network_chat->send_packet_verify_req(network_mgr->get_player_key(), id);

Lobby_Scene:
    network_lobby->create();
    std::string channel_server_ip;
    std::string channel_server_port;
    configurator::get_value("channel_server_ip", channel_server_ip);
    configurator::get_value("channel_server_port", channel_server_port);
    network_lobby->connect(CHANNEL_SERVER_IP, CHANNEL_SEFVER_PORT);
    network_lobby->send_packet_join_req(network_mgr->get_player_key(), id);
    
    do {
        Sleep(5000);

        if (accept_friend_match_)
            goto Loading_Scene;

        int branch = random_generator::get_random_int(0, 6);

        switch (branch) {
        case 0:
            network_chat->send_packet_chat_normal(id, "normal chat");
            break;

        case 1:
        {
            char temp[5] = "";
            itoa(random_generator::get_random_int(0, 999), temp, 10);

            network_chat->send_packet_chat_whisper(id, temp, "whisper chat");
            break;
        }

        case 2:
        {
            channel_server::basic_info info;

            char temp[5] = "";
            itoa(random_generator::get_random_int(0, 999), temp, 10);

            info.set_id(temp);
            network_lobby->send_packet_friend_req(channel_server::packet_friends_req_req_type_SEARCH, info);
            break;
        }

        case 3:
        {
            channel_server::basic_info info;

            char temp[5] = "";
            itoa(random_generator::get_random_int(0, 999), temp, 10);

            info.set_id(temp);
            network_lobby->send_packet_friend_req(channel_server::packet_friends_req_req_type_DEL, info);
            break;
        }

        case 4:
            goto Loading_Scene;
            break;
            
        case 5:
        {
            /*char temp[5] = "";
            itoa(random_generator::get_random_int(0, 999), temp, 10);

            friend_match_id_ = temp;
            send_friend_match_ = false;

            goto Loading_Scene;*/
        }
        break;
        }

    } while (true);
#pragma endregion
    
#pragma region Loading_Scene
Loading_Scene:
    do {
        Sleep(3000);

        static bool wait_result = false;

        if (wait_result)
        {
            if (state_ == CONNECT)
            {
                network_logic->create();
                std::string logic_server_ip;
                std::string logic_server_port;
                configurator::get_value("logic_server_ip", logic_server_ip);
                configurator::get_value("logic_server_port", logic_server_port);
                network_logic->connect(logic_server_ip, logic_server_port);
                network_logic->send_packet_enter_req(network_mgr->get_room_key(), network_mgr->get_player_key());

                wait_result = false;
                goto Play_Scene;
            }
            else if (state_ == DENY)
            {
                wait_result = false;
                goto Lobby_Scene;
            }
        
            continue;
        }

        if (!send_friend_match_)
        {
            network_lobby->send_packet_rank_game_req(true);
            wait_result = true;
        }
        else
        {
            if (!accept_friend_match_)
            {
                network_lobby->send_packet_play_friend_game_rel(
                    channel_server::packet_play_friends_game_rel_req_type_APPLY,
                    friend_match_id_
                );

                wait_result = true;
            }
            else
            {
                network_lobby->send_packet_play_friend_game_rel(
                    channel_server::packet_play_friends_game_rel_req_type_ACCEPT,
                    friend_match_id_
                );

                wait_result = true;
            }
        }
    } while (true);
#pragma endregion

#pragma region Play_Scene
Play_Scene:
    do {
        Sleep(3000);
        
        if (end_game_)
        {
            network_logic->destroy();
            
            network_lobby->create();
            network_lobby->connect(CHANNEL_SERVER_IP, CHANNEL_SEFVER_PORT);
            network_lobby->send_packet_join_req(network_mgr->get_player_key(), id);

            state_ = READY;

            end_game_ = false;

            goto Lobby_Scene;
        }

        int branch = random_generator::get_random_int(0, 100);

        //if (branch < 5)
            //network_logic->send_packet_disconnect_room_ntf();
    } while (true);
#pragma endregion
}
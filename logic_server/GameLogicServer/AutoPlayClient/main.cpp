#include "pre_header.h"
#include "logger.h"
#include "network_manager.h"
#include "chat_session.h"
#include "logic_session.h"
#include "channel_session.h"
#include "random_generator.h"

int main(int argc, char* argv[])
{
    //char *ip = argv[1];
    
    int behaviour_timer = 3000;

    std::string id = "2";
    std::string pwd = "2";

    std::queue<int> friend_list;


    logger::is_debug_mode(true);

    // Winsock Settings
    WSADATA wsa_data;
    
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        logger::print("WSAStartup - Failed");
        return 0;
    }  
    
    network_mgr->init_singleton();

    logger::print("Initialize Network");

    // 분기 1 : 로그인
    if (!network_mgr->try_login(id, pwd))
    {
        logger::print("Failed Login");
        return 0;
    }

    logger::print("Success Login");

#pragma region 분기 2 : 로비 화면

lobby:
    if (network_chat->connect(CHAT_SERVER_IP, CHAT_SERVER_PORT))
    {
        logger::print("Connect Failed - Chat Server");
        return 0;
    }
    else
        logger::print("Connect Chat Server");

    if (network_lobby->connect(CHANNEL_SERVER_IP, CHANNEL_SERVER_PORT))
    {
        logger::print("Connect Failed - Channel Server");
        return 0;
    }
    else
        logger::print("Connect Channel Server");

    network_chat->send_packet_verify_req(network_mgr->get_player_key(), id);
    network_lobby->send_packet_join_req(network_mgr->get_player_key(), id);

    while (true)
    {
        int branch = random_generator::get_random_int(0, 1);

        switch (branch)
        {
        case 0:
            network_chat->send_packet_chat_normal(id, "hi! I'm ai");
            break;

        case 1:
            {
                std::string target_id = "";
                target_id += random_generator::get_random_int(0, 1000);

                network_chat->send_packet_chat_whisper(id, target_id, "whisper test!");
            }
            break;

        case 2: network_lobby->send_packet_rank_game_req(true);
            break;

        case 3: 
            {
                char temp[10] = "";
                itoa(random_generator::get_random_int(0, 1000), temp, 10);
            
                network_lobby->send_packet_play_friend_game_rel(channel_server::packet_play_friends_game_rel_req_type_APPLY, temp);
            }
            break;
        
        case 4:
            {
                channel_server::basic_info info;

                char temp[10] = "";
                itoa(random_generator::get_random_int(0, 1000), temp, 10);
                info.set_id(temp);

                network_lobby->send_packet_friend_req(channel_server::packet_friends_req_req_type_ADD, info);
            }
            break;

        case 5:
            {
                channel_server::basic_info info;

                network_lobby->send_packet_friend_req(channel_server::packet_friends_req_req_type_DEL, info);
            }
            break;

        }

        Sleep(behaviour_timer);
    }

#pragma endregion 

#pragma region 분기 3.5 : 매칭 수락 화면

friend_match:
            int asd;


#pragma endregion

#pragma region 분기 3 : 로직 화면

    logic:
    

#pragma endregion

    network_mgr->release_singleton();
    

    WSACleanup();

    return 0;
}
#include "client.h"
#include "protocol.h"
#include <boost/thread.hpp>

#define SERVER_IP "127.0.0.1"

enum MENU
{
    LOBBY_LOGIN = 1,
    LOBBY_LOGOUT,
    PLAY,
    PLAY_WITH_FRIENDS,
    SEARCH,
    ADD,
    DEL,
    ACCEPT,
    DENY
};
int main()
{
    boost::asio::io_service io_service;

    boost::asio::ip::tcp::endpoint endpoint(
        boost::asio::ip::address::from_string(SERVER_IP),
        PORT_NUMBER
    );
    packet_handler handler;
    client clnt(io_service,handler);
    clnt.client_connect(endpoint);
    boost::thread t1(boost::bind(&boost::asio::io_service::run, &io_service));
    char user_id[200];
    char token[200];
    memset(token, 0, sizeof(token));
    
    while (1)
    {
        
        std::cout << "1. LOGIN\n2. LOGOUT\n3. PLAY\n4. PLAY_WITH_FRIENDS\n5. SEARCH\n6. ADD\n7. DEL\n8. ACCEPT\n9. DENY\n" << std::endl;
        int menu = 0;
        char yes_or_no;
        std::cin >> menu;
        system("cls");
        switch (menu)
        {
            case MENU::LOBBY_LOGIN:
            {
                std::cout << "token : ";
                std::cin >> token;
                clnt.post_join_message(token);
            }
                break;
            case MENU::LOBBY_LOGOUT:
            {
                std::cout << "logout" << std::endl;
                clnt.post_logout_message(token);
                clnt.socket().shutdown(clnt.socket().shutdown_both);
                clnt.socket().close();
            }
                break;
            case MENU::PLAY:
            {
                std::cout << "play" << std::endl;
                clnt.post_play_message();
            }
                break;
            case MENU::PLAY_WITH_FRIENDS:
            {
                memset(user_id, 0, sizeof(user_id));
                std::cout << "play with friends:  ";
                std::cin >> user_id;
                clnt.post_play_with_friends_message(user_id);
            }
                break;
            case MENU::SEARCH:
            {
                memset(user_id, 0, sizeof(user_id));
                std::cout << "search:  " << std::endl;
                std::cin >> user_id;
                clnt.post_search_message(user_id);
            }
                break;
            case MENU::ADD:
            {
                memset(user_id, 0, sizeof(user_id));
                std::cout << "add:  " << std::endl;
                std::cin >> user_id;
                clnt.post_add_message(user_id);
            }
                break;
            case MENU::DEL:
            {
                memset(user_id, 0, sizeof(user_id));
                std::cout << "del:  " << std::endl;
                std::cin >> user_id;
                clnt.post_del_message(user_id);

            }
                break;
            case MENU::ACCEPT:
            {
                clnt.post_accept_message();
                break;
            }
            case MENU::DENY:
            {
                clnt.post_deny_message();
                break;
            }
            default:
            {
            
            }
                break;
        }
        std::cout << "Other Command? Y/N" << std::endl;
        std::cin >> yes_or_no;
        if (yes_or_no == 'y' || yes_or_no == 'Y')
        {
            system("cls");
            continue;
        }
        else if (yes_or_no == 'N' || yes_or_no == 'n')
        {
            clnt.socket().close();
            return 0;
        }
    }
    std::cout << "匙飘况农 立加 辆丰" << std::endl;
    getchar();
    return 0;
}
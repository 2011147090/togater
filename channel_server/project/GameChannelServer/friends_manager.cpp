#include "friends_manager.h"




friends_manager::friends_manager(redispp::Connection & redis_connector, packet_handler & handler)
    : redis_connector_(redis_connector)
    , packet_handler_(handler)
{
}

friends_manager::~friends_manager()
{
    user_id_map_.clear();
}

bool friends_manager::lobby_login_process(session *request_session, const char * packet, const int packet_size)
{
    if (request_session->get_socket().is_open())
    {
        join_request message;
        packet_handler_.decode_message(message, packet, packet_size);
        std::string token = message.token();
        auto user_id = redis_connector_.get(token);
        if (user_id.result().is_initialized())
        {
            join_response response_message;
            game_history *history = response_message.mutable_history();
            /* DB query 대체 */
            int rating = 0;
            int battle_history = 0;
            int win = 0;
            int lose = 0;
            int friends_count = 0;
            std::string name = user_id.result().value() + "0001";
            // friends list 받아와서 넣어주기 (id 만) string
            
            request_session->set_user_info(rating,battle_history,win,lose,user_id.result().value());
            request_session->set_token(token.c_str());

            for (int i = 0; i < friends_count; i++)
            {
               basic_info *friends_id = response_message.add_friends_list();
               friends_id->set_id(name);
            }
            add_id_in_user_map(request_session, request_session->get_user_id());
            history->set_total_games(battle_history);
            history->set_rating_score(packet_handler_.check_rating(rating));
            history->set_win(win);
            history->set_lose(lose);
            //friends string list
            unsigned char incoding_size = packet_header_size + response_message.ByteSize();
            char *incoding_data = packet_handler_.incode_message(response_message);
            request_session->post_send(false, incoding_size, incoding_data);
            request_session->set_status(status::LOGIN);
            return true;
        }
        else
        {
            std::cout << "Invalid token value" << std::endl;
            request_session->get_socket().close();
            return false;
        }
    }
}

void friends_manager::del_redis_token(std::string token)
{
    auto check = redis_connector_.get(token);
    if (check.result().is_initialized())
    {
        redis_connector_.del(token);
    }
}

bool friends_manager::lobby_logout_process(session *request_session, const char *packet, const int packet_size)
{
    auto user_id = redis_connector_.get(request_session->get_token()); 
    if (user_id.result().is_initialized())
    {
        logout_response response_message;
        redis_connector_.del(request_session->get_token());
        request_session->set_status(status::LOGOUT);
        del_id_in_user_map(request_session->get_user_id());

        request_session->post_send(false, response_message.ByteSize() + packet_header_size, packet_handler_.incode_message(response_message));
        return true;
    }
    else
    {
        return false;
    }
    //packet 해석 후 map 컬렉션에서 삭제
}

void friends_manager::search_user(session * request_session, std::string target_id)
{
    friends_response message;
    
    session *target_session = find_id_in_user_map(target_id);
    if ( target_session == nullptr)
    {
        message.set_online(false);
        /*
        DB 쿼리로 상대방 정보 받아오기
        */
        /*
        찾으려하는 user가 Invalid 하면 에러 메시지 전송
        */
    }
    else
    {
        message.set_online(true);
        user_info  *target_user_info = message.mutable_friends_info();
        game_history *history = target_user_info->mutable_game_history_();
        basic_info *id = target_user_info->mutable_basic_info_();
        history->set_total_games(target_session->get_battle_history());
        history->set_lose(target_session->get_lose());
        history->set_win(target_session->get_win());
        history->set_rating_score(packet_handler_.check_rating(target_session->get_rating()));
        id->set_id(target_session->get_user_id());
    }

    request_session->post_send(false, message.ByteSize() + packet_header_size, packet_handler_.incode_message(message));

    // 클라이언트에게 전송
    // 없는 유저라면 requset_session 에게 없는 사용자라는 경고 문구 리턴
}

void friends_manager::add_friends(session * request_session, std::string target_id)
{
    std::cout << request_session->get_user_id() << "가 " << target_id << "를 추가 요청" << std::endl;
    //DB 쿼리로 target_id 저장 (notify 성격)
}

void friends_manager::del_friends(session * request_session, std::string target_id)
{
    std::cout << request_session->get_user_id() << "가 " << target_id << "를 삭제 요청" << std::endl;
    //DB 쿼리로 target_id 삭제 (notify 성격)
}

void friends_manager::process_friends_function(session * request_session, const char * packet, const int packet_size)
{
    friends_request message;
    packet_handler_.decode_message(message, packet, packet_size);
    switch (message.type())
    {
    case friends_add:
    {
        add_friends(request_session, message.mutable_target_info()->id());
        return;
    }
    case friends_del:
    {
        del_friends(request_session, message.mutable_target_info()->id());
        return;
    }
    case friends_search:
    {
        search_user(request_session, message.mutable_target_info()->id());
        return;
    }
    default:
        break;
    }
}

session* friends_manager::find_id_in_user_map(std::string target_id)
{
    auto iter = user_id_map_.find(target_id);

    if (iter != user_id_map_.end())
    {
        return iter->second;
    }
    else
    {
        return nullptr;
    }
}

void friends_manager::del_id_in_user_map(std::string target_id)
{
    auto iter = user_id_map_.find(target_id);
    if (iter != user_id_map_.end())
    {
        user_id_map_.erase(target_id);
    }
    return;
}

void friends_manager::add_id_in_user_map(session * request_session, std::string request_id)
{
    if (user_id_map_.find(request_id) == user_id_map_.end())
    {
        user_id_map_[request_id] = request_session;
    }
    return;
}




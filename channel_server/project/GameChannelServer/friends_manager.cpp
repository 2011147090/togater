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
        channel_serv::join_req message = packet_handler_.decode_join_req_message(packet, packet_size);
        std::string token = message.token();
        auto user_id = redis_connector_.get(token);
        if (user_id.result().is_initialized())
        {
            user_id_map_[user_id.result().value()] = request_session;
            /* DB query 대체 */
            int rating = 0;
            int battle_history = 100;
            int win = 100;
            int lose = 0;
            // friends list 받아와서 넣어주기 (id 만) string
            request_session->set_user_info(rating,user_id.result().value());

            channel_serv::join_ans answer_message;
            channel_serv::user_info *user = answer_message.mutable_my_info();
            user->set_battle_history(battle_history);
            user->set_rating(packet_handler_.check_rating(rating));
            user->set_win(win);
            user->set_lose(lose);
            user->set_user_id(request_session->get_user_id);
            //friends string list
            unsigned char incoding_size = packet_header_size + answer_message.ByteSize();
            char *incoding_data = packet_handler_.incode_message(answer_message);
            request_session->post_send(false, incoding_size, incoding_data);
            
            return true;
        }
    }
    else
    {
        std::cout << "Invalid token value" << std::endl;
        request_session->get_socket().close();
        return false;
    }
}

bool friends_manager::lobby_logout_process(session *request_session, const char *packet, const int packet_size)
{
    channel_serv::logout_ntf message = packet_handler_.decode_logout_nt_message(packet, packet_size);
    auto user_id = redis_connector_.get(message.token()); 
    if (user_id.result().is_initialized())
    {
        user_id_map_.erase(user_id.result().value());
        request_session->get_socket().close();
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
    bool on = true;
    // DB 쿼리로 target_id 의 기본정보 받고
    if (check_user_on_off_status == nullptr)
    {
        on = false;
    }
    // map 컬렉션에 있는지 확인 -> on/off 확인
    // 클라이언트에게 전송
    // 없는 유저라면 requset_session 에게 없는 사용자라는 경고 문구 리턴
}

void friends_manager::add_friends(session * request_session, std::string target_id)
{
    //DB 쿼리로 target_id 저장 (notify 성격)
}

void friends_manager::del_friends(session * request_session, std::string target_id)
{
    //DB 쿼리로 target_id 삭제 (notify 성격)
}

void friends_manager::process_friends_function(session * request_session, const char * packet, const int packet_size)
{
    channel_serv::friends_req message;
    message = packet_handler_.decode_friends_req_message(packet, packet_size);
    switch (message.req())
    {
    case channel_serv::friends_req::F_REQ::friends_req_F_REQ_ADD:
    {
        add_friends(request_session, message.user_id());
    }
    case channel_serv::friends_req::F_REQ::friends_req_F_REQ_DEL:
    {
        del_friends(request_session, message.user_id());
    }
    case channel_serv::friends_req::F_REQ::friends_req_F_REQ_SEARCH:
    {
        search_user(request_session, message.user_id());
    }
    default:
        break;
    }
}

session * friends_manager::check_user_on_off_status(std::string target_id)
{
    return nullptr;
}

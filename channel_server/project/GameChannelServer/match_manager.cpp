#include "match_manager.h"



match_manager::match_manager(packet_handler & packet_handler, friends_manager & friends_manager, redis_connector & redis_connection)
    : packet_handler_(packet_handler)
    , friends_manager_(friends_manager)
    , redis_connection_(redis_connection)
{ }

match_manager::~match_manager()
{ }

/*
랭크 게임 매칭
 *랭크 게임 매칭에 필요한 패킷은 게임을 원하는 사용자의 정보만 필요하다.
 레이팅 점수값을 매칭큐에 삽입 (매칭큐는 레이팅 점수를 토대로 몇가지 구간을 나누어 구분한다)
 각 구간 매칭큐에 매칭을 요구하는 세션이 삽입되면 해당 큐에 이미 대기자가 있으면 바로 꺼내서 매칭을 시켜준다.
 매칭 프로세스는 먼저 룸을 개설하고 개설한 룸번호롤 레디스에 입력 
 이때 각 사용자에게 매칭이 완료되었다는 패킷을 보낸다. 매칭완료 패킷은 방번호와 상대방 정보 송신

*/
void match_manager::process_matching(session *request_session, const char *packet, const int data_size)
{
    match_request message;
    packet_handler_.decode_message(message, packet, data_size);
    rating rating_name_ = packet_handler_.check_rating(request_session->get_rating());

    set_matching_que(request_session, rating_name_);
}


/*
친구와 게임하기 매칭
 *친구와 게임하기의 패킷 전송과정
 1. 플레이하고싶은 상대에게 먼저 대결신청을하고 (서버에서 recv 필드를 바꾸어서 전송)
 2. 대결은 신청받은 수신자는 Y/N를 선택하여 서버에 전송한다.
   3-1. Y 일 경우에는 룸을 개설해서 각 플레이어에게 전송
   3-2. N 일 경우에는 대결신청자에게 거절 메세지를 보낸다.

*/
void match_manager::process_matching_with_friends(session *request_session, const char *packet, const int data_size)
{
    match_with_friends_relay relay_message;
    error_report error_message;
    
    packet_handler_.decode_message(relay_message, packet, data_size);
    session *recv_session = friends_manager_.find_id_in_user_map(relay_message.target_id());

    switch (relay_message.type())
    {
    case match_with_friends_relay::ACCEPT:
    {
        if (recv_session == nullptr && request_session->get_status() != status::MATCH_RECVER && recv_session->get_status() != status::MATCH_APPLY)
        {
            relay_message.set_type(match_with_friends_relay::DENY);
            request_session->post_send(false, relay_message.ByteSize() + packet_header_size, packet_handler_.incode_message(relay_message));
            log_manager::get_instance()->get_logger()->warn("[Match Accept] [ERROR : Not found user or invalid status]");
            return;
        }
        
        match_complete match_message[2];
        user_info *match_user[2];
        session *player[2];
        
        std::string redis_room_key = generate_room_key();
        
        redis_connection_.set_reids_kv(redis_room_key, "0");

        for (int i = 0; i < 2; i++)
        {
            match_message[i].set_room_key(redis_room_key);
        }
        
        match_user[0] = match_message[1].mutable_opponent_player();
        match_user[1] = match_message[0].mutable_opponent_player();

        player[0] = request_session;
        player[1] = recv_session;

        for (int i = 0; i < 2; i++)
        {
            game_history *history = match_user[i]->mutable_game_history_();
            basic_info *id = match_user[i]->mutable_basic_info_();
            history->set_total_games(player[i]->get_battle_history());
            history->set_lose(player[i]->get_lose());
            history->set_win(player[i]->get_win());
            history->set_rating_score(player[i]->get_rating());
            id->set_id(player[i]->get_user_id());
        }
        
        for (int i = 0; i < 2; i++)
        {
            player[i]->set_status(status::MATCH_COMPLETE);
            player[i]->post_send(false, match_message[i].ByteSize() + packet_header_size, packet_handler_.incode_message(match_message[i]));
            friends_manager_.del_id_in_user_map(player[i]->get_user_id());
        }
        log_manager::get_instance()->get_logger()->info("[Match Accept] [Req ID : {0:s}] [Target Id : {1:s}]", request_session->get_user_id(), recv_session->get_user_id());
        log_manager::get_instance()->get_logger()->info("[Match Complete] [Req ID : {0:s}] [Target Id : {1:s}]", request_session->get_user_id(), recv_session->get_user_id());
    }
        break;
    case match_with_friends_relay::APPLY:
    {
        if (recv_session != nullptr && request_session->get_status() == status::LOGIN && recv_session->get_status() == status::LOGIN)
        {
            relay_message.set_target_id(request_session->get_user_id());
            request_session->set_status(status::MATCH_APPLY);
            recv_session->set_status(status::MATCH_RECVER);
            recv_session->post_send(false, relay_message.ByteSize() + packet_header_size, packet_handler_.incode_message(relay_message));
            log_manager::get_instance()->get_logger()->info("[Match Apply] [Req ID : {0:s}] [Target Id : {1:s}]", request_session->get_user_id(), recv_session->get_user_id());
            return;
        }
        else
        {
            relay_message.set_type(match_with_friends_relay::DENY);
            request_session->post_send(false, error_message.ByteSize() + packet_header_size, packet_handler_.incode_message(error_message));
            log_manager::get_instance()->get_logger()->warn("[Match Apply] [ERROR : Not found user or invalid status]");
        }
    }
        break;
    case match_with_friends_relay::DENY:
    {
        if (recv_session != nullptr && request_session->get_status() == status::MATCH_RECVER && recv_session->get_status() == status::MATCH_REQUEST)
        {
            relay_message.set_target_id(request_session->get_user_id());
            request_session->set_status(status::LOGIN);
            recv_session->set_status(status::LOGIN);
            recv_session->post_send(false, relay_message.ByteSize() + packet_header_size, packet_handler_.incode_message(relay_message));
            log_manager::get_instance()->get_logger()->info("[Match Deny] [Req ID : {0:s}] [Target Id : {1:s}]", request_session->get_user_id(), recv_session->get_user_id());
            return;
        }
        else
        {
            error_message.set_error_string("Not Found or You do not have permission");
            request_session->post_send(false, error_message.ByteSize() + packet_header_size, packet_handler_.incode_message(error_message));
            log_manager::get_instance()->get_logger()->warn("[Match Deny] [ERROR : Not found user or invalid status]");
        }
    }
        break;
    default:
        break;
    }
}

void match_manager::rematching_que(session * request_session)
{
    rating rating_name = packet_handler_.check_rating(request_session->get_rating());
    if (request_session->get_status() == status::MATCH_REQUEST)
    {
        for (int i = bronze; i < MAX_RATING; ++i)
        {
            if (i != rating_name)
            {
                set_matching_que(request_session, (rating)i);
            }
        }
        log_manager::get_instance()->get_logger()->info("[Rematching] [Rematch Complete]");
    }
    else if (request_session->get_status() == status::MATCH_COMPLETE)
    {
        log_manager::get_instance()->get_logger()->info("[Don't Need Rematching] [In Game]");
        return;
    }
    else
    {
        log_manager::get_instance()->get_logger()->warn("[Rematching] [ERROR : invalid status]");
        return;
    }
}



void match_manager::set_matching_que(session * request_session, rating request_rating)
{
    request_session->set_status(status::MATCH_REQUEST);
    std::cout << "[MATCH_REQ] [USER ID : " << request_session->get_user_id() << " ] " << std::endl;
    rank_que_mtx[request_rating].lock();
    matching_que[request_rating].push_back(request_session);
    get_matching_que(matching_que[request_rating]);
    rank_que_mtx[request_rating].unlock();
    log_manager::get_instance()->get_logger()->info("[Match Request] [Req ID : {0:s}]",request_session->get_user_id());
}

void match_manager::get_matching_que(std::deque<session *> &target_que) //shared_resouce
{
    if (target_que.size() > 1)
    {
        session *player[2];

        player[0] = target_que.front();
        target_que.pop_front();
        
        
        if (!(player[0]->get_socket().is_open()) || (player[0]->get_status() != status::MATCH_REQUEST))
        {
            return;
        }

        
        player[1] = target_que.front();
        target_que.pop_front();
        

        if (!(player[1]->get_socket().is_open()) || (player[1]->get_status() != status::MATCH_REQUEST))
        {
            target_que.push_front(player[0]);
            return;
        }

        match_complete message[2];
        user_info *match_info[2];
        std::string redis_room_key = generate_room_key();

        
        redis_connection_.set_reids_kv(redis_room_key, "0");

        
        match_info[0] = message[1].mutable_opponent_player();
        match_info[1] = message[0].mutable_opponent_player();

        for (int i = 0; i < 2; i++)
        {
            message[i].set_room_key(redis_room_key);
            game_history *history = match_info[i]->mutable_game_history_();
            basic_info *id = match_info[i]->mutable_basic_info_();
            history->set_total_games(player[i]->get_battle_history());
            history->set_lose(player[i]->get_lose());
            history->set_win(player[i]->get_win());
            history->set_rating_score(packet_handler_.check_rating(player[i]->get_rating()));
            id->set_id(player[i]->get_user_id());
        }

        for (int i = 0; i < 2; i++)
        {
            player[i]->set_status(status::MATCH_COMPLETE);
            player[i]->post_send(false, message[i].ByteSize() + packet_header_size, packet_handler_.incode_message(message[i]));
            friends_manager_.del_id_in_user_map(player[i]->get_user_id());
        }

        log_manager::get_instance()->get_logger()->info("[Match Complete] [Player 1 : {0:s}] [Player 2: {1:s}]", player[0]->get_user_id(), player[1]->get_user_id());
    }
    else
    {
        log_manager::get_instance()->get_logger()->info("[Match Waiting] [Req ID : {0:s}]",target_que.front()->get_user_id());
        target_que.front()->set_timer_rematch(30);
    }
}


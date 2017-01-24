#include "match_manager.h"



match_manager::match_manager(packet_handler & packet_handler, friends_manager & friends_manager, redispp::Connection & redis_connection)
    : packet_handler_(packet_handler)
    , friends_manager_(friends_manager)
    , redis_connection_(redis_connection)
{ room_number = 0;}

match_manager::~match_manager()
{ }

/*
랭크 게임 매칭
 *랭크 게임 매칭에 필요한 패킷은 게임을 원하는 사용자의 정보만 필요하다.
 레이팅 점수값을 매칭큐에 삽입 (매칭큐는 레이팅 점수를 토대로 몇가지 구간을 나누어 구분한다)
 각 구간 매칭큐에 매칭을 요구하는 세션이 삽입되면 해당 큐에 이미 대기자가 있으면 바로 꺼내서 매칭을 시켜준다.
 매칭 프로세스는 먼저 룸을 개설하고 개설한 룸번호롤 레디스에 입력 
 이때 각 사용자에게 매칭이 완료되었다는 패킷을 보낸다. 매칭완료 패킷은 방번호와 

*/
void match_manager::process_matching(session *request_session, const char *packet, const int data_size)
{
    channel_serv::play_rank_game_req message;
    packet_handler_.decode_message(message, packet, data_size);
    channel_serv::RATING rating_name = message.mutable_my_info()->rating();

    set_matching_que(request_session, rating_name);
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
    channel_serv::play_friends_game_req message;
    channel_serv::error_msg error_message;
    
    packet_handler_.decode_message(message, packet, data_size);
    session *recv_session = friends_manager_.find_id_in_user_map(message.recv_id());

    switch (message.flag_id())
    {
    case channel_serv::play_friends_game_req::ACCEPT:
    {
        if (recv_session == nullptr && request_session->get_status() != status::MATCH_RECVER && recv_session->get_status() != status::MATCH_REQUEST)
        {
            error_message.set_error_message("Not Found");
            request_session->post_send(false, error_message.ByteSize() + packet_header_size, packet_handler_.incode_message(error_message));
            return;
        }

        channel_serv::matching_complete_ans match_message[2];
        channel_serv::user_info *match_user[2];
        session *player[2];
        unsigned int room_num = generate_room_info();
        char redis_room_key[100];
        sprintf(redis_room_key, "Room:%d", room_num);
        redis_connection_.set(redis_room_key, "0");

        for (int i = 0; i < 2; i++)
        {
            match_message[i].set_room_number(room_num);
        }
        
        match_user[0] = match_message[1].mutable_opponent_player();
        match_user[1] = match_message[0].mutable_opponent_player();

        player[0] = request_session;
        player[1] = recv_session;

        for (int i = 0; i < 2; i++)
        {
            match_user[i]->set_battle_history(player[i]->get_battle_history());
            match_user[i]->set_lose(player[i]->get_lose());
            match_user[i]->set_win(player[i]->get_win());
            match_user[i]->set_user_id(player[i]->get_user_id());
            match_user[i]->set_rating(packet_handler_.check_rating(player[i]->get_rating()));
        }
        
        for (int i = 0; i < 2; i++)
        {
            player[i]->set_status(status::MATCH_COMPLETE);
            player[i]->post_send(false, match_message[i].ByteSize() + packet_header_size, packet_handler_.incode_message(match_message[i]));
        }
    }
        break;
    case channel_serv::play_friends_game_req::APPLY:
    {
        if (recv_session != nullptr && request_session->get_status() == status::LOGIN && recv_session->get_status() == status::LOGIN)
        {
            message.set_recv_id(request_session->get_user_id());
            request_session->set_status(status::MATCH_REQUEST);
            recv_session->set_status(status::MATCH_RECVER);
            recv_session->post_send(false, message.ByteSize() + packet_header_size, packet_handler_.incode_message(message));
            return;
        }
        else
        {
            error_message.set_error_message("Not Found or already request, receive");
            request_session->post_send(false, error_message.ByteSize() + packet_header_size, packet_handler_.incode_message(error_message));
        }
    }
        break;
    case channel_serv::play_friends_game_req::DENY:
    {
        if (recv_session != nullptr && request_session->get_status() == status::MATCH_RECVER && recv_session->get_status() == status::MATCH_REQUEST)
        {
            message.set_recv_id(request_session->get_user_id());
            request_session->set_status(status::LOGIN);
            recv_session->set_status(status::LOGIN);
            recv_session->post_send(false, message.ByteSize() + packet_header_size, packet_handler_.incode_message(message));
            return;
        }
        else
        {
            error_message.set_error_message("Not Found or You do not have permission");
            request_session->post_send(false, error_message.ByteSize() + packet_header_size, packet_handler_.incode_message(error_message));
        }
    }
        break;
    default:
        break;
    }
}



void match_manager::set_matching_que(session * request_session, channel_serv::RATING request_rating)
{
    switch (request_rating)
    {
    case channel_serv::RATING::BRONZE:
        bronze_que.push_back(request_session);
        get_matching_que(bronze_que);
        break;
    case channel_serv::RATING::SILVER:
        silver_que.push_back(request_session);
        get_matching_que(silver_que);
        break;
    case channel_serv::RATING::GOLD:
        gold_que.push_back(request_session);
        get_matching_que(gold_que);
        break;
    case channel_serv::RATING::PLATINUM:
        platinum_que.push_back(request_session);
        get_matching_que(platinum_que);
        break;
    case channel_serv::RATING::DIAMOND:
        diamond_que.push_back(request_session);
        get_matching_que(diamond_que);
        break;
    case channel_serv::RATING::MASTER:
        master_que.push_back(request_session);
        get_matching_que(master_que);
        break;
    case channel_serv::RATING::CHAL:
        challenger_que.push_back(request_session);
        get_matching_que(challenger_que);
        break;
    default:
        break;
    }
}

void match_manager::get_matching_que(std::deque<session *> &target_que) //shared_resouce
{
    if (target_que.size() > 1)
    {
        session *player[2];

        player[0] = target_que.front();
        target_que.pop_front();
        if (player[0]->get_status() == status::LOGOUT)
        {
            return;
        }
        player[1] = target_que.front();
        target_que.pop_front();
        if (player[1]->get_status() == status::LOGOUT)
        {
            target_que.push_back(player[0]);
            return;
        }

        channel_serv::matching_complete_ans message[2];
        channel_serv::user_info *player_info[2];
        unsigned room_num = generate_room_info();
        char redis_room_key[100];
        sprintf(redis_room_key, "Room:%d", room_num);
        redis_connection_.set(redis_room_key, "0");

        message[0].set_room_number(room_num);
        message[1].set_room_number(room_num);
        
        player_info[0] = message[1].mutable_opponent_player();
        player_info[1] = message[0].mutable_opponent_player();

        for (int i = 0; i < 2; i++)
        {
            player_info[i]->set_battle_history(player[i]->get_battle_history());
            player_info[i]->set_lose(player[i]->get_lose());
            player_info[i]->set_win(player[i]->get_win());
            player_info[i]->set_user_id(player[i]->get_user_id());
            player_info[i]->set_rating(packet_handler_.check_rating(player[i]->get_rating()));
        }

        for (int i = 0; i < 2; i++)
        {
            player[i]->set_status(status::MATCH_COMPLETE);
            player[i]->post_send(false, message[i].ByteSize() + packet_header_size, packet_handler_.incode_message(message[i]));
        }
        //큐배분
    }
    else
    {
        //비동기 타이머 셋팅 큐가 안잡히는 하위 티어 애들과 매칭 잡아줌
    }
}


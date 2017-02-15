#include "match_manager.h"



match_manager::match_manager(packet_handler & packet_handler, friends_manager & friends_manager, redis_connector & redis_connection)
    : packet_handler_(packet_handler)
    , friends_manager_(friends_manager)
    , redis_connection_(redis_connection)
    , timer_value_(0)
{
    config::get_instance()->get_value("SESSION_CONFIG", "TIMER", timer_value_);
}

match_manager::~match_manager()
{ }

/*
��ũ ���� ��Ī
 *��ũ ���� ��Ī�� �ʿ��� ��Ŷ�� ������ ���ϴ� ������� ������ �ʿ��ϴ�.
 ������ �������� ��Īť�� ���� (��Īť�� ������ ������ ���� ��� ������ ������ �����Ѵ�)
 �� ���� ��Īť�� ��Ī�� �䱸�ϴ� ������ ���ԵǸ� �ش� ť�� �̹� ����ڰ� ������ �ٷ� ������ ��Ī�� �����ش�.
 ��Ī ���μ����� ���� ���� �����ϰ� ������ ���ȣ�� ���𽺿� �Է� 
 �̶� �� ����ڿ��� ��Ī�� �Ϸ�Ǿ��ٴ� ��Ŷ�� ������. ��Ī�Ϸ� ��Ŷ�� ���ȣ�� ���� ���� �۽�

*/
void match_manager::process_matching(session *request_session, const char *packet, const int data_size)
{
    match_request message;
    packet_handler_.decode_message(message, packet, data_size);
    rating rating_name_ = check_rating(request_session->get_rating());
    set_matching_que(request_session, rating_name_);
}


/*
ģ���� �����ϱ� ��Ī
 *ģ���� �����ϱ��� ��Ŷ ���۰���
 1. �÷����ϰ���� ��뿡�� ���� ����û���ϰ� (�������� recv �ʵ带 �ٲپ ����)
 2. ����� ��û���� �����ڴ� Y/N�� �����Ͽ� ������ �����Ѵ�.
   3-1. Y �� ��쿡�� ���� �����ؼ� �� �÷��̾�� ����
   3-2. N �� ��쿡�� ����û�ڿ��� ���� �޼����� ������.

*/
void match_manager::process_matching_with_friends(session *request_session, const char *packet, const int data_size)
{
    match_with_friends_relay relay_message;
    error_report error_message;
    
    packet_handler_.decode_message(relay_message, packet, data_size);
    session *recv_session = friends_manager_.find_id_in_user_map(relay_message.target_id());
    if (recv_session == request_session)
    {
        request_session->set_status(status::LOGIN);
        relay_message.set_type(match_with_friends_relay::DENY);
        request_session->wait_send(false, relay_message.ByteSize() + packet_header_size, packet_handler_.incode_message(relay_message));
        log_manager::get_instance()->get_logger()->warn("[Match] [ERROR : recv == req]");
        return;
    }
    switch (relay_message.type())
    {
    case match_with_friends_relay::ACCEPT:
    {
        if (recv_session != nullptr && request_session->get_status() == status::MATCH_RECVER && recv_session->get_status() == status::MATCH_APPLY)
        {
            log_manager::get_instance()->get_logger()->info("[Match Accept] -Req_id [{0:s}] -Target_id [{1:s}]", request_session->get_user_id(), recv_session->get_user_id());
            make_matching_and_send_complete(request_session, recv_session);
            return;
        }
        else
        {
            relay_message.set_type(match_with_friends_relay::DENY);
            request_session->wait_send(false, relay_message.ByteSize() + packet_header_size, packet_handler_.incode_message(relay_message));
            log_manager::get_instance()->get_logger()->warn("[Match Accept] [ERROR : Not found user or invalid status]");
        }
        break;
    }
    case match_with_friends_relay::APPLY:
    {
        if (recv_session != nullptr && request_session->get_status() == status::LOGIN && recv_session->get_status() == status::LOGIN)
        {
            relay_message.set_target_id(request_session->get_user_id());
            request_session->set_status(status::MATCH_APPLY);
            recv_session->set_status(status::MATCH_RECVER);
            recv_session->wait_send(false, relay_message.ByteSize() + packet_header_size, packet_handler_.incode_message(relay_message));
            log_manager::get_instance()->get_logger()->info("[Match Apply] -Req_id [{0:s}] -Target_id [{1:s}]", request_session->get_user_id(), recv_session->get_user_id());
            return;
        }
        else
        {
            relay_message.set_type(match_with_friends_relay::DENY);
            request_session->wait_send(false, relay_message.ByteSize() + packet_header_size, packet_handler_.incode_message(relay_message));
            log_manager::get_instance()->get_logger()->warn("[Match Apply] [ERROR : Not found user or invalid status]");
        }
        break;
    }
    case match_with_friends_relay::DENY:
    {
        if (recv_session != nullptr && request_session->get_status() == status::MATCH_RECVER && recv_session->get_status() == status::MATCH_APPLY)
        {
            relay_message.set_target_id(request_session->get_user_id());
            request_session->set_status(status::LOGIN);
            recv_session->set_status(status::LOGIN);
            recv_session->wait_send(false, relay_message.ByteSize() + packet_header_size, packet_handler_.incode_message(relay_message));
            log_manager::get_instance()->get_logger()->info("[Match Deny] -Req_id [{0:s}] -Target_id [{1:s}]", request_session->get_user_id(), recv_session->get_user_id());
            return;
        }
        else
        {
            error_message.set_error_string("Not Found or You do not have permission");
            request_session->wait_send(false, error_message.ByteSize() + packet_header_size, packet_handler_.incode_message(error_message));
            log_manager::get_instance()->get_logger()->warn("[Match Deny][ERROR] -Req_id [{0:s}] -Target_id [{1:s}]", request_session->get_user_id(), recv_session->get_user_id());
        }
        break;
    }
    default:
        break;
    }
}

void match_manager::make_matching_and_send_complete(session *player_1, session *player_2)
{
    session *player[2];
    player[0] = player_1;
    player[1] = player_2;

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
        history->set_rating_score(player[i]->get_rating());
        id->set_id(player[i]->get_user_id());
    }

    for (int i = 0; i < 2; i++)
    {
        player[i]->set_status(status::MATCH_COMPLETE);
        player[i]->wait_send(false, message[i].ByteSize() + packet_header_size, packet_handler_.incode_message(message[i]));
        friends_manager_.del_id_in_user_map(player[i]->get_user_id());
    }

    log_manager::get_instance()->get_logger()->info("[Match Complete] -Player_1 [{0:s}] -Player_2 [{1:s}]", player[0]->get_user_id(), player[1]->get_user_id());
}

void match_manager::set_matching_que(session * request_session, rating request_rating)
{
    rank_que_mtx[request_rating].lock();
    matching_que[request_rating].push_back(request_session);
    request_session->set_status(status::MATCH_REQUEST);
    if (get_matching_que(matching_que[request_rating]))
    {
        log_manager::get_instance()->get_logger()->info("[Match Request & complete] -Req_id:{0:s}", request_session->get_user_id());
    }
    else
    {
        if (timer_value_ > 0)
        {
            log_manager::get_instance()->get_logger()->info("[ReMatch Request] -Req_id:{0:s}", request_session->get_user_id());
            request_session->control_timer_rematch(timer_value_, true);
        }
    }
    rank_que_mtx[request_rating].unlock();
}

bool match_manager::rematching_start(session * request_session)
{
    if (request_session->get_status() == status::MATCH_REQUEST)
    {
        rank_que_mtx[shared_que].lock();
        matching_que[shared_que].push_back(request_session);
        get_matching_que(matching_que[shared_que]);
        rank_que_mtx[shared_que].unlock();
    }
    return true;
}

bool match_manager::get_matching_que(std::deque<session *> &target_que) //shared_resouce
{
    if (target_que.size() > 1)
    {
        session *player[2];

        player[0] = target_que.front();
        target_que.pop_front();
        
        
        if (!(player[0]->get_socket().is_open()) || (player[0]->get_status() != status::MATCH_REQUEST))
        {
            return false;
        }

        player[1] = target_que.front();
        target_que.pop_front();
        

        if (!(player[1]->get_socket().is_open()) || (player[1]->get_status() != status::MATCH_REQUEST) || (player[0] == player[1]))
        {
            target_que.push_front(player[0]);
            return false;
        }

        make_matching_and_send_complete(player[0],player[1]);
        return true;
    }
    else
    {
        return false;
    }
}

rating match_manager::check_rating(const int rating)
{
    if (rating < 300) return rating::bronze;
    else if (rating < 400) return rating::silver;
    else if (rating < 500) return rating::gold;
    else if (rating < 600) return rating::platinum;
    else if (rating < 700) return rating::diamond;
    else if (rating < 900) return rating::master;
    else return rating::challenger;
}
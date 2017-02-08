#include "channel_session.h"
#include "logic_session.h"
#include "network_manager.h"

void channel_session::handle_send(channel_server::message_type msg_type, const protobuf::Message& message)
{
    thread_sync sync;

    boost::array<BYTE, BUFSIZE> send_buf;
    boost::array<BYTE, BUFSIZE> recv_buf;
    boost::array<BYTE, 128> recv_buf_ori;

    MESSAGE_HEADER send_header;
    send_header.size = message.ByteSize();
    send_header.type = msg_type;

    CopyMemory(send_buf.begin(), (void*)&send_header, message_header_size);

    message.SerializeToArray(send_buf.begin() + message_header_size, send_header.size);

    send(socket_, (char*)send_buf.begin(), message_header_size + send_header.size, 0);
    int size = recv(socket_, (char*)recv_buf_ori.begin(), 128, 0);

    int remain_size = size;
    int process_size = 0;

    do
    {
        MESSAGE_HEADER recv_header;
        CopyMemory(&recv_header, recv_buf_ori.begin() + process_size, message_header_size);
        CopyMemory(recv_buf.begin(), recv_buf_ori.begin() + process_size, recv_header.size + message_header_size);

        process_size += message_header_size + recv_header.size;
        remain_size -= process_size;

        switch (recv_header.type)
        {
        case channel_server::JOIN_ANS:
        {
            channel_server::packet_join_ans message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_join_ans(message);
        }
        break;

        case channel_server::LOGOUT_ANS:
        {
            channel_server::packet_logout_ans message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_logout_ans(message);
        }
        break;

        case channel_server::FRIENDS_ANS:
        {
            channel_server::packet_friends_ans message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_friend_ans(message);
        }
        break;

        case channel_server::PLAY_RANK_ANS:
        {
            channel_server::packet_play_rank_game_ans message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_rank_game_ans(message);
        }
        break;

        case channel_server::PLAY_FRIENDS_REL:
        {
            channel_server::packet_play_friends_game_rel message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_play_friend_game_rel(message);
        }
        break;

        case channel_server::MATCH_COMPLETE:
        {
            channel_server::packet_matching_complete_ans message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_matching_complete_ans(message);
        }
        break;

        case channel_server::ERROR_MSG:
        {
            channel_server::packet_error_message message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_error_message(message);
        }
        break;

        }
    } while (remain_size > 0);
}

void channel_session::process_packet_join_ans(channel_server::packet_join_ans packet)
{
    thread_sync sync;

    if (packet.success() == false)
        return;

    for (int i = 0; i < packet.friends_list_size(); ++i)
    {
        const channel_server::basic_info& info = packet.friends_list(i);
        
        /*game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                game_manager::add_friend_in_list,
                game_mgr,
                info.id()
            )
        );*/
    }

    if (packet.has_history())
    {
        const channel_server::game_history& history = packet.history();

        /*game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                game_manager::set_history,
                game_mgr,
                history.win(),
                history.lose(),
                history.rating_score()
            )
        );*/

        network_mgr->set_player_history(history);
    }
}

void channel_session::process_packet_logout_ans(channel_server::packet_logout_ans packet)
{
    thread_sync sync;

}

void channel_session::process_packet_friend_ans(channel_server::packet_friends_ans packet)
{
    thread_sync sync;

    const channel_server::user_info& friend_info = packet.friends_info();
    const channel_server::game_history& friend_history = friend_info.game_history_();
    const channel_server::basic_info& friend_basic_info = friend_info.basic_info_();
    
    friend_history.rating_score();
    friend_history.total_games();
    friend_history.win();
    friend_history.lose();
    
    switch (packet.type())
    {
    case channel_server::packet_friends_ans_ans_type::packet_friends_ans_ans_type_SEARCH_SUCCESS:
    {
        channel_server::basic_info info;
//        info.set_id(game_mgr->friend_text_field->getString());
        this->send_packet_friend_req(channel_server::packet_friends_req_req_type_ADD, info);
    }
    break;

    case channel_server::packet_friends_ans_ans_type::packet_friends_ans_ans_type_ADD_SUCCESS:
        /*game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                game_manager::add_friend_in_list,
                game_mgr,
                packet.mutable_friends_info()->mutable_basic_info_()->id()
            )
        );*/
        break;

    case channel_server::packet_friends_ans_ans_type::packet_friends_ans_ans_type_DEL_SUCCESS:
        /*game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                game_manager::del_friend_in_list,
                game_mgr,
                packet.mutable_friends_info()->mutable_basic_info_()->id()
            )
        );*/
        break;
    }
}

void channel_session::process_packet_rank_game_ans(channel_server::packet_play_rank_game_ans packet)
{
    thread_sync sync;

}

void channel_session::process_packet_play_friend_game_rel(channel_server::packet_play_friends_game_rel packet)
{
    thread_sync sync;

    switch(packet.type())
    {
    case channel_server::packet_play_friends_game_rel::APPLY:
        /*game_mgr->accept_friend_match_ = true;
        game_mgr->friend_match_id_ = packet.target_id();

        game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                lobby_scene::show_friend_match_pop_up,
                game_mgr->lobby_scene_
            )
        );*/

        break;

    case channel_server::packet_play_friends_game_rel::DENY:
        /*game_mgr->accept_friend_match_ = false;
                
        cocos2d::CCDirector::getInstance()->popScene();*/
        break;
    }
}

void channel_session::process_packet_matching_complete_ans(channel_server::packet_matching_complete_ans packet)
{
    thread_sync sync;
    
    /*game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            game_manager::set_opponent_info,
            game_mgr,
            packet.mutable_opponent_player()->mutable_basic_info_()->id(),
            packet.mutable_opponent_player()->mutable_game_history_()->win(),
            packet.mutable_opponent_player()->mutable_game_history_()->lose(),
            packet.mutable_opponent_player()->mutable_game_history_()->rating_score()
        )
    );*/

    network_mgr->set_room_key(packet.room_key());

    /*game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(game_manager::start_game, game_mgr)
    );*/
    
    send_packet_matching_confirm();
}

void channel_session::process_packet_error_message(channel_server::packet_error_message packet)
{
    thread_sync sync;
}

void channel_session::send_packet_join_req(std::string key, std::string id)
{
    thread_sync sync;

    channel_server::packet_join_req packet;

    packet.set_token(key);
    packet.set_id(id);

    this->handle_send(channel_server::JOIN_REQ, packet);
}

void channel_session::send_packet_logut_req(bool flag)
{
    thread_sync sync;

    channel_server::packet_logout_req packet;
    packet.set_none(flag);

    this->handle_send(channel_server::LOGOUT_REQ, packet);
}

void channel_session::send_packet_friend_req(channel_server::packet_friends_req::req_type type, channel_server::basic_info info)
{
    thread_sync sync;
    
    channel_server::packet_friends_req packet;
    packet.set_type(type);
    packet.mutable_target_info()->set_id(info.id());

    this->handle_send(channel_server::FRIENDS_REQ, packet);
}

void channel_session::send_packet_rank_game_req(bool flag)
{
    thread_sync sync;

    channel_server::packet_play_rank_game_req packet;
    packet.set_none(flag);

    this->handle_send(channel_server::PLAY_RANK_REQ, packet);
}

void channel_session::send_packet_play_friend_game_rel(channel_server::packet_play_friends_game_rel::req_type type, std::string target_id)
{
    thread_sync sync;

    channel_server::packet_play_friends_game_rel packet;
    packet.set_type(type);
    packet.set_target_id(target_id);

    this->handle_send(channel_server::PLAY_FRIENDS_REL, packet);
}

void channel_session::send_packet_matching_confirm()
{
    thread_sync sync;

    channel_server::packet_matching_confirm packet;

    this->handle_send(channel_server::MATCH_CONFIRM, packet);

    this->disconnect();
}
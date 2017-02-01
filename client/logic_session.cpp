#include "logic_session.h"
#include "game_manager.h"
#include "network_manager.h"

bool logic_session::create()
{
    thread_sync sync;

    is_connected_ = false;

    if (socket_ == nullptr)
        socket_ = new tcp::socket(io_service_);

    return true;
}

bool logic_session::destroy()
{
    thread_sync sync;

    disconnect();

    is_connected_ = false;

    work_thread_->join();

    if (socket_ != nullptr)
    {
        delete socket_;
        socket_ = nullptr;
    }

    return true;
}

void logic_session::handle_send(logic_server::message_type msg_type, const protobuf::Message& message)
{
    thread_sync sync;

    MESSAGE_HEADER header;
    header.size = message.ByteSize();
    header.type = msg_type;

    int buf_size = 0;
    buf_size = message_header_size + message.ByteSize();

    CopyMemory(send_buf_.begin(), (void*)&header, message_header_size);

    message.SerializeToArray(send_buf_.begin() + message_header_size, header.size);

    boost::system::error_code error;
    socket_->write_some(boost::asio::buffer(send_buf_, message_header_size + header.size), error);
}

void logic_session::handle_read()
{
    while (true)
    {
        if (!is_socket_open())
            break;

        boost::system::error_code error;

        int i = 0;

        socket_->receive(boost::asio::buffer(recv_buf_), i, error);

        if (error)
            return;
        
        thread_sync sync;

        MESSAGE_HEADER message_header;

        CopyMemory(&message_header, recv_buf_.begin(), message_header_size);

        switch (message_header.type)
        {
        case logic_server::ENTER_ANS:
        {
            logic_server::packet_enter_ans message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_enter_ans(message);
        }
        break;

        case logic_server::PROCESS_TURN_REQ:
        {
            logic_server::packet_process_turn_req message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_process_turn_req(message);
        }
        break;

        case logic_server::PROCESS_TURN_NTF:
        {
            logic_server::packet_process_turn_ntf message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_process_turn_ntf(message);

            break;
        }

        case logic_server::GAME_STATE_NTF:
        {
            logic_server::packet_game_state_ntf message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_game_state_ntf(message);
        }
        break;

        case logic_server::PROCESS_CHECK_CARD_NTF:
        {
            logic_server::packet_process_check_card_ntf message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_process_check_card_ntf(message);
        }
        break;

        }
    }
}


void logic_session::process_packet_enter_ans(logic_server::packet_enter_ans packet)
{
    thread_sync sync;

    logic_server::packet_enter_ans recevie_packet;
}

void logic_session::process_packet_process_turn_req(logic_server::packet_process_turn_req packet)
{
    thread_sync sync;

    game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            game_manager::opponent_turn_end, game_mgr,
            packet.my_money(),
            packet.opponent_money()
        )
    );
}

void logic_session::process_packet_process_turn_ntf(logic_server::packet_process_turn_ntf packet)
{
    thread_sync sync;

    game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            game_manager::new_turn, game_mgr,
            packet.public_card_number_1(),
            packet.public_card_number_2(),
            packet.opponent_card_number(),
            packet.remain_money(),
            packet.my_money(),
            packet.opponent_money()
        )
    );
}

void logic_session::process_packet_process_check_card_ntf(logic_server::packet_process_check_card_ntf packet)
{
    thread_sync sync;

    game_mgr->get_scheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            game_manager::check_public_card, game_mgr,
            )
    );
}

void logic_session::process_packet_game_state_ntf(logic_server::packet_game_state_ntf packet)
{
    thread_sync sync;

    if (packet.state() == 1)
    {
        game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                game_manager::start_game, game_mgr,
                )
        );
    }
    else if (packet.state() == 2)
    {
        game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                main_scene::end, game_mgr->scene_,
                )
        );
    }
}

void logic_session::send_packet_enter_req(std::string room_key, std::string player_key)
{
    thread_sync sync;

    logic_server::packet_enter_req enter_req_packet;
    enter_req_packet.set_room_key(room_key);
    enter_req_packet.set_player_key(player_key);

    this->handle_send(logic_server::ENTER_REQ, enter_req_packet);
}

void logic_session::send_packet_process_turn_ans(int money)
{
    thread_sync sync;

    logic_server::packet_process_turn_ans process_turn_packet;
    process_turn_packet.set_money(money);

    this->handle_send(logic_server::PROCESS_TURN_ANS, process_turn_packet);
}

void logic_session::send_packet_disconnect_room_ntf()
{
    thread_sync sync;

    logic_server::packet_disconnect_room_ntf disconnect_room_packet;
    disconnect_room_packet.set_room_key(network_mgr->get_room_key());

    this->handle_send(logic_server::DISCONNECT_ROOM, disconnect_room_packet);
}
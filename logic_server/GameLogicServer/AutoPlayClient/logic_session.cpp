#include "logic_session.h"
#include "network_manager.h"

void logic_session::handle_send(logic_server::message_type msg_type, const protobuf::Message& message)
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
        case logic_server::ENTER_ANS:
        {
            logic_server::packet_enter_ans message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_enter_ans(message);
        }
        break;

        case logic_server::PROCESS_TURN_REQ:
        {
            logic_server::packet_process_turn_req message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_process_turn_req(message);
        }
        break;

        case logic_server::PROCESS_TURN_NTF:
        {
            logic_server::packet_process_turn_ntf message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_process_turn_ntf(message);

            break;
        }

        case logic_server::GAME_STATE_NTF:
        {
            logic_server::packet_game_state_ntf message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_game_state_ntf(message);
        }
        break;

        case logic_server::PROCESS_CHECK_CARD_NTF:
        {
            logic_server::packet_process_check_card_ntf message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_process_check_card_ntf(message);
        }
        break;
        }

    } while (remain_size > 0);
}

void logic_session::process_packet_enter_ans(logic_server::packet_enter_ans packet)
{
    thread_sync sync;

    logic_server::packet_enter_ans recevie_packet;

    network_logic->send_packet_game_state_ntf();
}

void logic_session::process_packet_process_turn_req(logic_server::packet_process_turn_req packet)
{
    thread_sync sync;

}

void logic_session::process_packet_process_turn_ntf(logic_server::packet_process_turn_ntf packet)
{
    thread_sync sync;

}

void logic_session::process_packet_process_check_card_ntf(logic_server::packet_process_check_card_ntf packet)
{
    thread_sync sync;

}

void logic_session::process_packet_game_state_ntf(logic_server::packet_game_state_ntf packet)
{
    thread_sync sync;

    if (packet.state() == 1)
    {
        /*game_mgr->get_scheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                game_manager::start_game, game_mgr,
            )
        );*/
    }
    else if (packet.state() == 2)
    {
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

void logic_session::send_packet_game_state_ntf()
{
    thread_sync sync;

    logic_server::packet_game_state_ntf packet;

    packet.set_state(0);
    packet.set_win_player_key("");

    this->handle_send(logic_server::GAME_STATE_NTF, packet);
}
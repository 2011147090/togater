#include "chat_session.h"
#include "network_manager.h"
#include "logger.h"

void chat_session::handle_send(chat_server::message_type msg_type, const protobuf::Message& message)
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
        case chat_server::VERIFY_ANS:
        {
            chat_server::packet_verify_ans message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_verify_ans(message);
        }
        break;

        case chat_server::LOGOUT_ANS:
        {
            chat_server::packet_logout_ans message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_pacekt_logout_ans(message);
        }
        break;

        case chat_server::NORMAL:
        {
            chat_server::packet_chat_normal message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_chat_normal(message);
        }
        break;

        case chat_server::WHISPER:
        {
            chat_server::packet_chat_whisper message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_chat_whisper(message);
        }
        break;

        case chat_server::ROOM:
        {
            chat_server::packet_chat_room message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_chat_room(message);
        }
        break;

        case chat_server::NOTICE:
        {
            chat_server::packet_chat_notice message;

            if (false == message.ParseFromArray(recv_buf.begin() + message_header_size, recv_header.size))
                break;

            process_packet_chat_notice(message);
        }
        break;

        }
    } while (remain_size > 0);
}

void chat_session::process_packet_verify_ans(chat_server::packet_verify_ans packet)
{
    thread_sync sync;

    if (packet.is_successful())
        logger::print("[chat server] : verify_ans is success!");
    else
        logger::print("[chat server] : verify_ans is failed!");
}

void chat_session::process_pacekt_logout_ans(chat_server::packet_logout_ans packet)
{
    thread_sync sync;

    if (packet.is_successful())
        logger::print("[chat server] : logout_ans is success!");
    else
        logger::print("[chat server] : logout_ans is failed!");
}

void chat_session::process_packet_chat_normal(chat_server::packet_chat_normal packet)
{
    thread_sync sync;

    std::string message = packet.user_id() + " : " + packet.chat_message();
 
    logger::print("[c] " + message);
}

void chat_session::process_packet_chat_whisper(chat_server::packet_chat_whisper packet)
{
    thread_sync sync;

    if (packet.user_id() == "Error")
    {
        std::string message = packet.user_id() + " : " + packet.chat_message();

        logger::print("[w] " + message);
    }
    else
    {
        logger::print("[w] not found user");
    }
}

void chat_session::process_packet_chat_room(chat_server::packet_chat_room packet)
{
    thread_sync sync;

    std::string message = packet.user_id() + " : " + packet.chat_message();

    logger::print("[r] " + message);
}

void chat_session::process_packet_chat_notice(chat_server::packet_chat_notice packet)
{
    thread_sync sync;
    
    std::string message = packet.user_id() + " : " + packet.chat_message();

    logger::print("[n] " + message);
}

void chat_session::send_packet_verify_req(std::string player_key, std::string id)
{
    thread_sync sync;

    chat_server::packet_verify_req packet;
    packet.set_key_string(player_key);
    packet.set_value_user_id(id);

    logger::print("[chat server] : verify_req / key : " + player_key + " / id : " + id);

    this->handle_send(chat_server::VERIFY_REQ, packet);
}

void chat_session::send_packet_logout_req(std::string player_id)
{
    thread_sync sync;

    chat_server::packet_logout_req packet;
    packet.set_user_id(player_id);

    logger::print("[chat server] : logout_req / id :" + player_id);
    
    this->handle_send(chat_server::LOGOUT_REQ, packet);
}

void chat_session::send_packet_enter_match_ntf(std::string target_id)
{
    thread_sync sync;
    
    chat_server::packet_enter_match_ntf packet;

    packet.set_opponent_id(target_id);

    logger::print("[chat server] : enter_match_ntf / with " + target_id);

    this->handle_send(chat_server::ENTER_MATCH_NTF, packet);
}

void chat_session::send_packet_leave_match_ntf()
{
    thread_sync sync;

    chat_server::packet_leave_match_ntf packet;

    logger::print("[chat server] : leave_match_ntf");

    this->handle_send(chat_server::LEAVE_MATCH_NTF, packet);
}

void chat_session::send_packet_chat_normal(std::string id, std::string message)
{
    thread_sync sync;

    chat_server::packet_chat_normal packet;
    packet.set_chat_message(message);
    packet.set_user_id(id);

    logger::print("[chat server] : send_chat_normal / " + message);

    this->handle_send(chat_server::NORMAL, packet);
}

void chat_session::send_packet_chat_whisper(std::string id, std::string target_id, std::string message)
{
    thread_sync sync;

    chat_server::packet_chat_whisper packet;
    packet.set_chat_message(message);
    packet.set_user_id(id);
    packet.set_target_id(target_id);

    logger::print("[chat server] : send_chat_whisper / To " + target_id + " / " + message);

    this->handle_send(chat_server::WHISPER, packet);
}

void chat_session::send_packet_chat_room(std::string id, std::string message)
{
    thread_sync sync;

    chat_server::packet_chat_room packet;
    packet.set_chat_message(message);
    packet.set_user_id(id);

    logger::print("[chat server] : send_chat_room / " + message);

    this->handle_send(chat_server::ROOM, packet);
}
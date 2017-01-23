#include "chat_session.h"
#include "game_manager.h"
#include "network_manager.h"

bool chat_session::create()
{
    thread_sync sync;

    is_connected_ = false;

    socket_ = new tcp::socket(io_service_);

    return true;
}

bool chat_session::destroy()
{
    thread_sync sync;

    is_connected_ = false;

    work_thread_->join();

    if (socket_ != NULL)
        delete socket_;

    return true;
}

void chat_session::handle_send(chat_server::message_type msg_type, const protobuf::Message& message)
{
    thread_sync sync;

    MESSAGE_HEADER header;
    header.size = message.ByteSize();
    header.type = msg_type;

    int buf_size = 0;
    buf_size = message_header_size + message.ByteSize();

    CopyMemory(send_buf_.begin(), (void*)&header, message_header_size);

    message.SerializeToArray(send_buf_.begin() + message_header_size, header.size);

    socket_->write_some(boost::asio::buffer(send_buf_));
}

void chat_session::handle_read()
{
    while (true)
    {
        if (!is_socket_open())
            break;

        socket_->receive(boost::asio::buffer(recv_buf_));

        thread_sync sync;

        MESSAGE_HEADER message_header;

        CopyMemory(&message_header, recv_buf_.begin(), message_header_size);

        switch (message_header.type)
        {
        case chat_server::NORMAL:
        {
            chat_server::packet_chat_normal message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_chat_normal(message);
        }
        break;

        case chat_server::VERIFY_RES:
        {
            chat_server::packet_verify_res message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_verify_res(message);
        }
        break;

        }
    }
}

void chat_session::send_packet_verify_req(std::string player_key, std::string id)
{
    thread_sync sync;

    chat_server::packet_verify_req packet;
    packet.set_key_string(player_key);
    packet.set_value_user_id(id);

    this->handle_send(chat_server::VERIFY_REQ, packet);
}

void chat_session::send_packet_chat_normal(std::string id, std::string message)
{
    thread_sync sync;

    chat_server::packet_chat_normal packet;
    packet.set_chat_message(message);
    packet.set_user_id(id);

    this->handle_send(chat_server::NORMAL, packet);
}

void chat_session::process_packet_verify_res(chat_server::packet_verify_res packet)
{
    thread_sync sync;
}

void chat_session::process_packet_chat_normal(chat_server::packet_chat_normal packet)
{
    thread_sync sync;

    game_mgr->scheduler_->performFunctionInCocosThread(
        CC_CALLBACK_0(
            game_manager::add_lobby_chat, game_mgr,
            packet.user_id(), packet.chat_message()
        )
    );
}
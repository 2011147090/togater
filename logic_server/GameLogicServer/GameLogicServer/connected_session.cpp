#include "preHeaders.h"
#include "connected_session.h"
#include "log_manager.h"

connected_session::connected_session(boost::asio::io_service& io_service) : socket_(io_service)
{}

bool connected_session::handle_check_keep_alive()
{
    boost::system::error_code error;

    if (error)
        return false;

    return true;
}

void connected_session::handle_send(logic_server::message_type msg_type, const protobuf::Message& message)
{
    MESSAGE_HEADER header;
    header.size = message.ByteSize();
    header.type = msg_type;

    CopyMemory(send_buf_.begin(), (void*)&header, message_header_size);

    message.SerializeToArray(send_buf_.begin() + message_header_size, header.size);

    boost::system::error_code error;
    socket_.write_some(boost::asio::buffer(send_buf_, message_header_size + header.size), error);

    if (error)
        system_log->error(error.message());
}

void connected_session::shut_down()
{
    socket_.shutdown(boost::asio::socket_base::shutdown_receive);
    socket_.close();
}

std::string connected_session::get_player_key()
{
    return player_key_;
}

void connected_session::handle_read(const boost::system::error_code& error, size_t /*bytes_transferred*/)
{
    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(recv_buf_),
            boost::bind(&connected_session::handle_read, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

        MESSAGE_HEADER message_header;

        CopyMemory(&message_header, recv_buf_.begin(), message_header_size);

        switch (message_header.type)
        {
        case logic_server::ENTER_REQ:
        {
            logic_server::packet_enter_req message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_enter_req(message);
        }
        break;

        case logic_server::PROCESS_TURN_ANS:
        {
            logic_server::packet_process_turn_ans message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_process_turn_ans(message);
        }
        break;

        case logic_server::DISCONNECT_ROOM:
        {
            logic_server::packet_disconnect_room_ntf message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_disconnect_room_ntf(message);
        }
        break;

        case logic_server::ECHO_NTF:
        {
            logic_server::packet_echo_ntf message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_echo_ntf(message);
        }
        break;
        }
    }
    else
        system_log->error("handle_read_error:{}", error.message());
}

bool connected_session::is_connected()
{
    if (socket_.is_open())
        return true;

    return false;
}

connected_session::pointer connected_session::create(boost::asio::io_service& io_service)
{
    return connected_session::pointer(new connected_session(io_service));
}

tcp::socket& connected_session::get_socket()
{
    return socket_;
}

void connected_session::start()
{
    socket_.async_read_some(boost::asio::buffer(recv_buf_),
        boost::bind(&connected_session::handle_read, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}
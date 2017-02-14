#include "pre_headers.h"
#include "connected_session.h"
#include "log_manager.h"

connected_session::connected_session(boost::asio::io_service& io_service) : socket_(io_service), safe_disconnect_(true), enter_room_(false), is_accept_(false)
{
}

void connected_session::handle_send(logic_server::message_type msg_type, const protobuf::Message& message)
{
    //thread_sync sync;

    MESSAGE_HEADER header;
    header.size = message.ByteSize();
    header.type = msg_type;

    CopyMemory(send_buf_.begin(), (void*)&header, message_header_size);

    message.SerializeToArray(send_buf_.begin() + message_header_size, header.size);

    boost::system::error_code error;
    socket_.write_some(boost::asio::buffer(send_buf_, message_header_size + header.size), error);

    if (error)
        Log::WriteLog(_T("%s"), error.message());
}

void connected_session::shut_down()
{
   // thread_sync sync;

    if (socket_.is_open())
    {
        socket_.shutdown(boost::asio::socket_base::shutdown_receive);
        socket_.close();
    }
}

std::string connected_session::get_player_key()
{
    //thread_sync sync;

    return player_key_;
}

std::string connected_session::get_room_key()
{
    //thread_sync sync;

    return room_key_;
}

void connected_session::handle_read(const boost::system::error_code& error, size_t buf_size)
{
    //thread_sync sync;

    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(recv_buf_),
            boost::bind(&connected_session::handle_read, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

        static boost::array<BYTE, BUFSIZE> packet_buf;

        int remain_size = buf_size;
        int process_size = 0;

        do
        {
            MESSAGE_HEADER message_header;
            CopyMemory(&message_header, recv_buf_.begin() + process_size, message_header_size);
            CopyMemory(packet_buf.begin(), recv_buf_.begin() + process_size, message_header.size + message_header_size);

            process_size += message_header_size + message_header.size;
            remain_size -= process_size;

            switch (message_header.type)
            {
            case logic_server::ENTER_REQ:
            {
                logic_server::packet_enter_req message;

                if (false == message.ParseFromArray(packet_buf.begin() + message_header_size, message_header.size))
                    break;

                process_packet_enter_req(message);
            }
            break;

            case logic_server::GAME_STATE_NTF:
            {
                logic_server::packet_game_state_ntf message;

                if (false == message.ParseFromArray(packet_buf.begin() + message_header_size, message_header.size))
                    break;

                porcess_packet_game_state_ntf(message);
            }
            break;

            case logic_server::PROCESS_TURN_ANS:
            {
                logic_server::packet_process_turn_ans message;

                if (false == message.ParseFromArray(packet_buf.begin() + message_header_size, message_header.size))
                    break;

                process_packet_process_turn_ans(message);
            }
            break;

            case logic_server::DISCONNECT_ROOM:
            {
                logic_server::packet_disconnect_room_ntf message;

                if (false == message.ParseFromArray(packet_buf.begin() + message_header_size, message_header.size))
                    break;

                process_packet_disconnect_room_ntf(message);
            }
            break;

            case logic_server::ECHO_NTF:
            {
                logic_server::packet_echo_ntf message;

                if (false == message.ParseFromArray(packet_buf.begin() + message_header_size, message_header.size))
                    break;

                process_packet_echo_ntf(message);
            }
            break;
            }
        } while (remain_size > 0);
    }
    else
    {
        Log::WriteLog(_T("handle_read_error: %s"), error.message().c_str());
        
        if (is_connected())
        {
            safe_disconnect_ = false;
            this->shut_down();
        }
    }
}

bool connected_session::is_connected()
{
    //thread_sync sync;

    if (socket_.is_open())
        return true;

    return false;
}

connected_session::pointer connected_session::create(boost::asio::io_service& io_service)
{
    //thread_sync sync;

    return connected_session::pointer(new connected_session(io_service));
}

tcp::socket& connected_session::get_socket()
{
    //thread_sync sync;

    return socket_;
}

void connected_session::start()
{
    //thread_sync sync;

    socket_.async_read_some(boost::asio::buffer(recv_buf_),
        boost::bind(&connected_session::handle_read, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));

    boost::asio::socket_base::keep_alive option;
    socket_.set_option(option);

    is_accept_ = true;
}

bool connected_session::is_safe_disconnect()
{
    //thread_sync sync;

    return safe_disconnect_;
}

bool connected_session::is_in_room()
{
   // thread_sync sync;

    return enter_room_;
}

void connected_session::set_room_state(bool is_enter)
{
    //thread_sync sync;

    enter_room_ = is_enter;
}

bool connected_session::accept_client()
{
    //thread_sync sync;

    return is_accept_;
}
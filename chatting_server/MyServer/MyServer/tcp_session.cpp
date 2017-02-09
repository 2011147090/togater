#include "log_manager.h"
#include "redis_connector.h"

#include "tcp_server.h"
#include "tcp_session.h"


// ---------- public ----------
tcp_session::tcp_session(int session_id, boost::asio::io_service& io_service, tcp_server* server)
    :socket_(io_service), session_id_(session_id), server_(server), user_id_("")
{
}

tcp_session::~tcp_session()
{
}

void tcp_session::post_verify_ans(bool is_successful)
{
    chat_server::packet_verify_ans verify_message;
    verify_message.set_is_successful(is_successful);
    
    MESSAGE_HEADER header;
    
    header.size = verify_message.ByteSize();
    header.type = chat_server::VERIFY_ANS;


    boost::array<BYTE, 1024>* send_buffer = new boost::array<BYTE, 1024>;

    CopyMemory(send_buffer->begin(), (void*)&header, message_header_size);
    verify_message.SerializeToArray(send_buffer->begin() + message_header_size, header.size);

    post_send(false, message_header_size + header.size, send_buffer->begin());
}

void tcp_session::post_logout_ans(bool is_successful)
{
    chat_server::packet_logout_ans logout_message;
    logout_message.set_is_successful(is_successful);

    MESSAGE_HEADER header;

    header.size = logout_message.ByteSize();
    header.type = chat_server::LOGOUT_ANS;


    boost::array<BYTE, 1024>* send_buffer = new boost::array<BYTE, 1024>;

    CopyMemory(send_buffer->begin(), (void*)&header, message_header_size);
    logout_message.SerializeToArray(send_buffer->begin() + message_header_size, header.size);

    post_send(false, message_header_size + header.size, send_buffer->begin());
}

void tcp_session::post_whisper_error()
{
    chat_server::packet_chat_whisper error_message;
    error_message.set_user_id("Error");
    error_message.set_target_id(user_id_);
    error_message.set_chat_message("");

    MESSAGE_HEADER header;

    header.size = error_message.ByteSize();
    header.type = chat_server::WHISPER;


    boost::array<BYTE, 1024>* send_buffer = new boost::array<BYTE, 1024>;

    CopyMemory(send_buffer->begin(), (void*)&header, message_header_size);
    error_message.SerializeToArray(send_buffer->begin() + message_header_size, header.size);

    post_send(false, message_header_size + header.size, send_buffer->begin());
}

void tcp_session::post_send(const bool immediate, const int size, BYTE* data)
{
    if (immediate == false)
        send_data_queue_.push_back(data);
    
    if (immediate == false && send_data_queue_.size() > 1)
        return;

    boost::asio::async_write(socket_, boost::asio::buffer(data, size),
        boost::bind(&tcp_session::handle_write, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)
    );
}

void tcp_session::post_receive()
{
    socket_.async_read_some(boost::asio::buffer(receive_buffer_),
        boost::bind(&tcp_session::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)
    );
}


// ---------- private ----------
void tcp_session::handle_write(const boost::system::error_code& error, size_t bytes_transferred)
{
    MESSAGE_HEADER* message_header = (MESSAGE_HEADER*)send_data_queue_.front();
    

    switch (message_header->type)
    {
    case chat_server::VERIFY_ANS:
        {
            chat_server::packet_verify_ans verify_message;
            verify_message.ParseFromArray(send_data_queue_.front() + message_header_size, message_header->size);

            if (socket_.is_open() && !(verify_message.is_successful()))
                server_->get_io_service().post(server_->strand_close_.wrap(boost::bind(&tcp_server::close_session, server_, session_id_)));
        }
        break;

    case chat_server::LOGOUT_ANS:
        {
            chat_server::packet_logout_ans logout_message;
            logout_message.ParseFromArray(send_data_queue_.front() + message_header_size, message_header->size);

            if (socket_.is_open() && logout_message.is_successful())
                server_->get_io_service().post(server_->strand_close_.wrap(boost::bind(&tcp_server::close_session, server_, session_id_)));
        }
        break;

    case chat_server::NORMAL:
        break;

    case chat_server::WHISPER:
        break;

    case chat_server::ROOM:
        break;
    
    case chat_server::NOTICE:
        break;
    }

    send_data_queue_.pop_front();
    if (send_data_queue_.empty() == false)
    {
        BYTE* data = send_data_queue_.front();
        MESSAGE_HEADER* packet = (MESSAGE_HEADER*)data;
        post_send(true, packet->size + message_header_size, data);
    }
}

void tcp_session::handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {
        server_->get_io_service().post(server_->strand_receive_.wrap(boost::bind(&tcp_server::process_packet, server_, session_id_, bytes_transferred, receive_buffer_.begin())));

        post_receive();
    }
    else
    {
        if (error == boost::asio::error::eof)
            LOG_INFO << "Disconnected with client" << std::endl;
        else
            LOG_WARN << "handle_receive() : Error No: " << error.value() << " Error Message: " << error.message();
        
        server_->get_io_service().post(server_->strand_close_.wrap(boost::bind(&tcp_server::close_session, server_, session_id_)));
    }
}


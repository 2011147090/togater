#include "tcp_session.h"
#include "tcp_server.h"


// ---------- public ----------
tcp_session::tcp_session(int session_id, boost::asio::io_service& io_service, tcp_server* server)
    :socket_(io_service), session_id_(session_id), server_(server), user_id_("")
{
}

tcp_session::~tcp_session()
{
}

void tcp_session::post_send(const bool immediate, const int size, BYTE* data)
{
    if (immediate == false)
    {
        send_data_queue_.push_back(data);
    }
    
    if (immediate == false && send_data_queue_.size() > 1)
    {
        return;
    }

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
    if (error)
    {
        if (error == boost::asio::error::eof)
        {
            std::cout << "Disconnected with client" << std::endl;
        }
        else
        {
            std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
        }
    }
    else
    {
        server_->process_packet(session_id_, bytes_transferred, receive_buffer_.begin());

        post_receive();
    }
}
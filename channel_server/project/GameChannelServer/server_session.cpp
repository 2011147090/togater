#include "server_session.h"
#include "log_manager.h"
#include "channel_server.h"


session::session(int session_id, boost::asio::io_service &io_service, tcp_server* p_channel_serv)
    : session_id_(session_id)
    , socket_(io_service)
    , channel_serv_(p_channel_serv)
{
}

session::~session()
{
    while (send_data_queue_.empty() == false)
    {
        delete[] send_data_queue_.front();
        send_data_queue_.pop_front();
    }
}

void session::init()
{
    packet_buffer_mark_ = 0;
    memset(token_.data(), 0, TOKEN_SIZE);
}

void session::post_receive()
{
    socket_.async_read_some(boost::asio::buffer(receive_buffer_), boost::bind(&session::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void session::post_send(const bool b_immediately, const int n_size, char * p_data)
{
    char* p_send_data = nullptr;

    if (b_immediately == false)
    {
        p_send_data = new char[n_size];
        memcpy(p_send_data, p_data, n_size);

        send_data_queue_.push_back(p_send_data);
    }
    else
    {
        p_send_data = p_data;
    }

    if (b_immediately == false && send_data_queue_.size() > 1)
    {
        return;
    }

    boost::asio::async_write(socket_, boost::asio::buffer(p_send_data, n_size),
        boost::bind(&session::handle_write, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void session::handle_write(const boost::system::error_code & error, size_t bytes_transferred)
{
    delete[] send_data_queue_.front();
    send_data_queue_.pop_front();

    if (send_data_queue_.empty() == false)
    {
        char *p_data = send_data_queue_.front();
        packet_header *p_header = (packet_header *)p_data;
        post_send(true, p_header->size, p_data);
    }
}

void session::handle_receive(const boost::system::error_code & error, size_t bytes_transferred)
{
    if (error)
    {
        if (error == boost::asio::error::eof)
        {
            std::cout << "클라이언트와 연결이 끊어졌습니다" << std::endl; //log
        }
        else
        {
            std::cout << "error No: " << error.value() << "error Message: " << error.message() << std::endl; //log
        }
        channel_serv_->close_session(session_id_); //server 클래스 작성하고 주석 풀기
    }
    else
    {
        memcpy(&packet_buffer_[packet_buffer_mark_], receive_buffer_.data(), bytes_transferred);

        int n_packet_data = packet_buffer_mark_ + bytes_transferred;
        int n_read_data = 0;

        while (n_packet_data > 0) 
        {
            if (n_packet_data < packet_header_size)
            {
                break;
            }

            packet_header *p_header = (packet_header *)&packet_buffer_[n_read_data];

            if (p_header->size + packet_header_size <= n_packet_data)
            {
                channel_serv_->process_packet(session_id_, &packet_buffer_[n_read_data]);

                n_packet_data -= (p_header->size + packet_header_size);
                n_read_data += (p_header->size + packet_header_size);
            }
            else
            {
                break;
            }
        }

        if (n_packet_data > 0)
        {
            char TempBuffer[MAX_RECEIVE_BUFFER_LEN] = { 0, };
            memcpy(&TempBuffer[0], &packet_buffer_[n_read_data], n_packet_data);
            memcpy(&packet_buffer_[0], &TempBuffer[0], n_packet_data);
        }

        packet_buffer_mark_ = n_packet_data;

        post_receive();
    }
}

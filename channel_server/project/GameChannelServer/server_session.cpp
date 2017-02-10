#include "server_session.h"
#include "channel_server.h"

session::session(int session_id, boost::asio::io_service &io_service, tcp_server* p_channel_serv,const int token_size, const int max_buffer_len)
    : session_id_(session_id)
    , socket_(io_service)
    , channel_serv_(p_channel_serv)
    , match_timer_(io_service)
    , con_timer_(io_service)
    , token_size_(token_size)
    , max_buffer_len_(max_buffer_len)
{
    stat_ = status::WAIT;
    token_ = new char[token_size_];
    receive_buffer_ = new char[max_buffer_len_];
    temp_buffer_ = new char[max_buffer_len_];
    packet_buffer_ = new char[max_buffer_len_ * 2];
    cancel_flag = false;
}

session::~session()
{
    while (send_data_queue_.empty() == false)
    {
        delete[] send_data_queue_.front();
        send_data_queue_.pop_front();
    }
    delete[] receive_buffer_;
    delete[] temp_buffer_;
    delete[] packet_buffer_;
}

void session::init()
{
    packet_buffer_mark_ = 0;
}

void session::flush()
{
    control_timer_conn(0, false);
    control_timer_rematch(0, false);
}

void session::wait_receive()
{
    socket_.async_read_some(
        boost::asio::buffer(receive_buffer_,max_buffer_len_),
        boost::bind(
            &session::handle_receive, 
            this, 
            boost::asio::placeholders::error, 
            boost::asio::placeholders::bytes_transferred
        )
    );
}

void session::wait_send(const bool immediately, const int send_data_size, char * send_data)
{
    if (immediately == false)
    {
        send_data_queue_.push_back(send_data);
    }


    if (immediately == false && send_data_queue_.size() > 1)
    {
        return;
    }

    boost::asio::async_write(
        socket_, boost::asio::buffer(send_data, send_data_size),   
        boost::bind(
            &session::handle_write,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred
        )
    );
}

void session::rematch(const boost::system::error_code & error)
{
    if (channel_serv_->rematching_request(this) == false)
    {
        control_timer_rematch(5, true);
    }
}

void session::check_status(const boost::system::error_code & error)
{
    if (stat_ == status::CONN || stat_ == status::MATCH_COMPLETE || stat_ == status::LOGOUT)
    {
        channel_serv_->close_session(session_id_, true);
    }
}

void session::control_timer_conn(int sec, bool is_set)                                            // true -> set , false -> cancle
{
    if (is_set && (sec > 0))
    {
        con_timer_.expires_from_now(std::chrono::seconds(sec));
        con_timer_.async_wait(boost::bind(&session::check_status, this, boost::asio::placeholders::error));
    }
    else
    {
        log_manager::get_instance()->get_logger()->critical("conn timer done session_id {0:d}", session_id_);
        con_timer_.cancel();
    }
}

void session::control_timer_rematch(int sec, bool is_set)
{
    if (is_set && (sec > 0))
    {
        match_timer_.expires_from_now(std::chrono::seconds(sec));
        match_timer_.async_wait(boost::bind(&session::rematch, this, boost::asio::placeholders::error));
    }
    else
    {
        log_manager::get_instance()->get_logger()->critical("[*rematch timer*] done usr_id {0:s}", this->get_user_id());
        match_timer_.cancel();
    }
}

void session::handle_write(const boost::system::error_code & error, size_t bytes_transferred)
{
    delete[] send_data_queue_.front();
    send_data_queue_.pop_front();

    if (stat_ == status::MATCH_COMPLETE || stat_ == status::LOGOUT) 
    {
        while (send_data_queue_.empty() == false)
        {
            delete[] send_data_queue_.front();
            send_data_queue_.pop_front();
        }
        //control_timer_conn(60, true);
        return;
    }

    if (send_data_queue_.empty() == false)
    {
        char *p_data = send_data_queue_.front();
        packet_header *p_header = (packet_header *)p_data;
        wait_send(true, p_header->size + packet_header_size, p_data);
    }
}

void session::handle_receive(const boost::system::error_code & error, size_t bytes_transferred) // bytes_transferred 크기가 MAX_BUFFER_LEN * 2 보다 클수도 있다
{
    if (error)
    {
        if (error == boost::asio::error::eof)
        {
            log_manager::get_instance()->get_logger()->info("[Client Socket Close] [Socket Info : -session_id {0:d}]",session_id_);
            channel_serv_->close_session(session_id_, false);
        }
        else if (error == boost::asio::error::connection_reset)
        {
            log_manager::get_instance()->get_logger()->info("[Connection terminate] [Socket Info : -session_id {0:d}]", session_id_);
            channel_serv_->close_session(session_id_, true);
            control_timer_conn(0, false);
        }
        else
        {
            log_manager::get_instance()->get_logger()->warn("[Recv Error No : {0:d}] [Error Message : {1:s}] [User ID : {2:s}]", error.value(), error.message(), get_user_id());
        }
    }
    else
    {
        memcpy(&packet_buffer_[packet_buffer_mark_], receive_buffer_, bytes_transferred);

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
            memset(&temp_buffer_[0], 0, max_buffer_len_);
            memcpy(&temp_buffer_[0], &packet_buffer_[n_read_data], n_packet_data);
            memcpy(&packet_buffer_[0], &temp_buffer_[0], n_packet_data);
        } 

        packet_buffer_mark_ = n_packet_data;

        wait_receive();
    }
}

#include "tcp_client.h"


// ---------- public ----------
tcp_client::tcp_client(boost::asio::io_service& io_service)
    :io_service_(io_service), socket_(io_service)
{
    InitializeCriticalSectionAndSpinCount(&lock_, 4000);
}

tcp_client::~tcp_client()
{
    EnterCriticalSection(&lock_);

    while (send_data_queue_.empty() == false)
    {
        delete[] send_data_queue_.front();
        send_data_queue_.pop_front();
    }

    LeaveCriticalSection(&lock_);

    DeleteCriticalSection(&lock_);
}

void tcp_client::connect(boost::asio::ip::tcp::endpoint endpoint)
{
    packet_buffer_mark_ = 0;

    socket_.async_connect(endpoint,
        boost::bind(&tcp_client::handle_connect, this,
            boost::asio::placeholders::error)
    );
}

void tcp_client::close()
{
    if (socket_.is_open())
    {
        socket_.close();
    }
}

void tcp_client::post_verify()
{
    chat_server::packet_verify_user verify_message;
    verify_message.set_key_string(key_);
    verify_message.set_value_user_id(user_id_);

    MESSAGE_HEADER header;

    header.size = verify_message.ByteSize();
    header.type = chat_server::VERIFY;

    CopyMemory(send_buffer_.begin(), (void*)&header, message_header_size);
    verify_message.SerializeToArray(send_buffer_.begin() + message_header_size, header.size);

    boost::asio::async_write(socket_, boost::asio::buffer(send_buffer_.begin(), message_header_size + header.size),
        boost::bind(&tcp_client::handle_verify, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)
    );
}

void tcp_client::post_send(const bool immediate, std::string message)
{
    chat_server::packet_chat_normal normal_message;
    normal_message.set_user_id(user_id_);
    normal_message.set_chat_message(message);

    MESSAGE_HEADER header;

    header.size = normal_message.ByteSize();
    header.type = chat_server::NORMAL;

    CopyMemory(send_buffer_.begin(), (void*)&header, message_header_size);
    normal_message.SerializeToArray(send_buffer_.begin() + message_header_size, header.size);


    BYTE* send_data;

    EnterCriticalSection(&lock_);

    if (immediate == false)
    {
        send_data = send_buffer_.begin();
        send_data_queue_.push_back(send_data);
    }
    else
    {
        send_data = send_buffer_.begin();
    }

    if (immediate || send_data_queue_.size() < 2)
    {
        boost::asio::async_write(socket_, boost::asio::buffer(send_data, message_header_size + header.size),
            boost::bind(&tcp_client::handle_write, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)
        );
    }

    LeaveCriticalSection(&lock_);
}

void tcp_client::post_send(const bool immediate, const int size, BYTE* data)
{
    BYTE* send_data = nullptr;

    EnterCriticalSection(&lock_);		// 락 시작

    if (immediate == false)
    {
        send_data = data;
        send_data_queue_.push_back(send_data);
    }
    else
    {
        send_data = data;
    }

    if (immediate || send_data_queue_.size() < 2)
    {
        boost::asio::async_write(socket_, boost::asio::buffer(send_data, size),
            boost::bind(&tcp_client::handle_write, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)
        );
    }

    LeaveCriticalSection(&lock_);		// 락 완료
}


// ---------- private ----------
void tcp_client::post_receive()
{
    socket_.async_read_some(
        boost::asio::buffer(receive_buffer_),
        boost::bind(&tcp_client::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)

    );
}

void tcp_client::handle_connect(const boost::system::error_code& error)
{
    if (!error)
    {
        std::cout << "서버 접속 성공" << std::endl;

        post_receive();
    }
    else
    {
        std::cout << "서버 접속 실패. error No: " << error.value() << " error Message: " << error.message() << std::endl;
    }
}

void tcp_client::handle_write(const boost::system::error_code& error, size_t bytes_transferred)
{
    EnterCriticalSection(&lock_);            // 락 시작
    send_data_queue_.pop_front();

    BYTE* data = nullptr;

    if (send_data_queue_.empty() == false)
    {
        data = send_data_queue_.front();
    }

    LeaveCriticalSection(&lock_);            // 락 완료


    if (data != nullptr)
    {
        MESSAGE_HEADER* header = (MESSAGE_HEADER*)data;
        post_send(true, header->size + message_header_size, data);
    }
}

void tcp_client::handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (error)
    {
        if (error == boost::asio::error::eof)
        {
            std::cout << "클라이언트와 연결이 끊어졌습니다" << std::endl;
        }
        else
        {
            std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
        }

        close();
    }
    else
    {
        process_packet(bytes_transferred);
        post_receive();
    }
}

void tcp_client::handle_verify(const boost::system::error_code& error, size_t bytes_transferred)
{
}

void tcp_client::process_packet(const int size)
{
    CopyMemory(&packet_buffer_, receive_buffer_.data(), size);

    MESSAGE_HEADER* message_header = (MESSAGE_HEADER*)packet_buffer_.begin();

    switch (message_header->type)
    {
    case chat_server::VERIFY:
        break;
    case chat_server::NORMAL:
    {
        chat_server::packet_chat_normal normal_message;
        
        normal_message.ParseFromArray(packet_buffer_.begin() + message_header_size, message_header->size);

        std::cout << normal_message.user_id() << "> ";
        std::cout << normal_message.chat_message() << std::endl;
    }
    break;
    case chat_server::WHISPER:
        break;
    case chat_server::ROOM:
        break;
    case chat_server::NOTICE:
        break;
    }
}
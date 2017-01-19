#include "tcp_server.h"


// ---------- public ----------
tcp_server::tcp_server(boost::asio::io_service& io_service)
    :acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
{
    is_accepting_ = false;
}

tcp_server::~tcp_server()
{
    for (int i = 0; i < session_list_.size(); i++)
    {
        if (session_list_[i]->get_socket().is_open())
        {
            session_list_[i]->get_socket().close();
        }

        delete session_list_[i];
    }
}

void tcp_server::init(const int max_session_count)
{
    for (int i = 0; i < max_session_count; i++)
    {
        tcp_session* session = new tcp_session(i, acceptor_.get_io_service(), this);
        session_list_.push_back(session);
        session_queue_.push_back(i);
    }
}

void tcp_server::start()
{
    std::cout << "Server Start..." << std::endl;

    post_accept();
}

void tcp_server::close_session(const int session_id)
{
    // ¸Ê<user_id, session*>¿¡¼­ Á¦°Å
    std::string session_user_id = session_list_[session_id]->get_user_id();
    connected_session_map_.erase(connected_session_map_.find(session_user_id));

    std::cout << "Client connection closed. session_id: " << session_id << std::endl;

    session_list_[session_id]->get_socket().close();
    session_queue_.push_back(session_id);

    if (is_accepting_ == false)
    {
        post_accept();
    }
}

void tcp_server::process_packet(const int session_id, const int size, BYTE* packet)
{
    MESSAGE_HEADER* message_header = (MESSAGE_HEADER*)packet;
    
    boost::array<BYTE, 1024>* send_data = new boost::array<BYTE, 1024>;
    CopyMemory(send_data, packet, size);

    switch (message_header->type)
    {
    case chat_server::NORMAL:
        for (int i = 0; i < session_list_.size(); i++)
        {
            if (session_list_[i]->get_socket().is_open())
            {
                session_list_[i]->post_send(false, size, send_data->begin());
            }
        }
        delete[] send_data;
        break;
    case chat_server::WHISPER:
        break;
    case chat_server::ROOM:
        break;
    case chat_server::NOTICE:
        break;
    }
}


// ---------- private ----------
bool tcp_server::post_accept()
{
    if (session_queue_.empty())
    {
        is_accepting_ = false;
        return false;
    }

    is_accepting_ = true;
    int session_id = session_queue_.front();

    session_queue_.pop_front();

    acceptor_.async_accept(session_list_[session_id]->get_socket(),
        boost::bind(&tcp_server::handle_accept, this,
            session_list_[session_id],
            boost::asio::placeholders::error)
    );

    return true;
}

void tcp_server::handle_accept(tcp_session* session, const boost::system::error_code& error)
{
    if (!error)
    {
        // ¸Ê<user_id, session*>¿¡ Ãß°¡
        connected_session_map_.insert(std::pair<std::string, tcp_session*>(session->get_user_id(), session));
        std::cout << "Client connection successed. session_id: " << session->get_session_id() << std::endl;

        session->post_receive();
        post_accept();
    }
    else
    {
        std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
    }
}
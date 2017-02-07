#include "tcp_server.h"
#include "redis_connector.h"


// ---------- public ----------
tcp_server::tcp_server(boost::asio::io_service& io_service)
    :io_service_(io_service),
        acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER)),
        strand_accept_(io_service), strand_close_(io_service)
        //, strand_receive_(io_service),  strand_send_(io_service)
{
    is_accepting_ = false;
    master_data_queue_.set_capacity(MAX_MASTER_BUFFER_LEN);
}

tcp_server::~tcp_server()
{
    for (int i = 0; i < session_list_.size(); i++)
    {
        if (session_list_[i]->get_socket().is_open())
            session_list_[i]->get_socket().close();
        
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

    io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
}

void tcp_server::close_session(const int session_id)
{
    std::string user_id = session_list_[session_id]->get_user_id();
    if (connected_session_map_.find(user_id) != connected_session_map_.end())
        connected_session_map_.erase(user_id);

    std::cout << "Client connection closed. session_id: " << session_id << std::endl;

    session_list_[session_id]->get_socket().close();
    session_queue_.push_back(session_id);

    if (is_accepting_ == false)
        post_accept();
}

void tcp_server::process_packet(const int session_id, const int size, BYTE* packet)
{
    MESSAGE_HEADER* message_header = (MESSAGE_HEADER*)packet;
    
    switch (message_header->type)
    {
    //사용자 인증
    case chat_server::VERIFY_REQ:
        {
            chat_server::packet_verify_req verify_message;
            verify_message.ParseFromArray(packet + message_header_size, message_header->size);
        
            std::string redis_key = verify_message.key_string();
            std::string redis_value = verify_message.value_user_id();

            if (redis_connector::get_instance()->get(redis_key) == redis_value)
            {
                session_list_[session_id]->set_user_id(redis_value);
                session_list_[session_id]->set_status(lobby);

                connected_session_map_.insert(std::pair<std::string, tcp_session*>(redis_value, session_list_[session_id]));

                session_list_[session_id]->post_verify_ans(true);
            }
            else
                session_list_[session_id]->post_verify_ans(false);

        }
        break;

    // 로그아웃
    case chat_server::LOGOUT_REQ:
        {
            chat_server::packet_logout_req logout_message;
            logout_message.ParseFromArray(packet + message_header_size, message_header->size);

            std::string user_id = logout_message.user_id();

            // 의미없는 if문
            if (user_id == session_list_[session_id]->get_user_id())
                session_list_[session_id]->post_logout_ans(true);
            else
                session_list_[session_id]->post_logout_ans(false);

        }
        break;


    // 방 입장
    case chat_server::ENTER_MATCH_NTF:
        {
            chat_server::packet_enter_match_ntf enter_match_message;
            enter_match_message.ParseFromArray(packet + message_header_size, message_header->size);

            std::string opponent_id = enter_match_message.opponent_id();

            auto iter = connected_session_map_.find(opponent_id);
            session_list_[session_id]->set_opponent_session(iter->second);
            session_list_[session_id]->set_status(room);
        }
        break;

    // 방 퇴장
    case chat_server::LEAVE_MATCH_NTF:
        {
            session_list_[session_id]->set_opponent_session(nullptr);
            session_list_[session_id]->set_status(lobby);
        }
        break;


    // 일반 채팅
    case chat_server::NORMAL:
        {
            boost::array<BYTE, 1024> send_data;
            CopyMemory(&send_data, packet, size);
            master_data_queue_.push_back(send_data);

            auto iter = connected_session_map_.begin();
            for (iter = connected_session_map_.begin(); iter != connected_session_map_.end(); ++iter)
            {
                if (iter->second->get_socket().is_open() && iter->second->get_status() == lobby)
                    iter->second->post_send(false, size, master_data_queue_.back().begin());
            }
        }
        break;

    // 귓속말
    case chat_server::WHISPER:
        {
            chat_server::packet_chat_whisper whisper_message;
            whisper_message.ParseFromArray(packet + message_header_size, message_header->size);

            boost::array<BYTE, 1024> send_data;
            CopyMemory(&send_data, packet, size);
            master_data_queue_.push_back(send_data);

            auto iter = connected_session_map_.find(whisper_message.target_id());
            if (session_list_[session_id]->get_socket().is_open())
            {
                session_list_[session_id]->post_send(false, size, master_data_queue_.back().begin());
                iter->second->post_send(false, size, master_data_queue_.back().begin());
            }
        }
        break;

    // 방 채팅
    case chat_server::ROOM:
        {
            boost::array<BYTE, 1024> send_data;
            CopyMemory(&send_data, packet, size);
            master_data_queue_.push_back(send_data);
            
            if (master_data_queue_.size() >= MAX_MASTER_BUFFER_LEN)
                std::cout << "FULL!" << std::endl;

            if (session_list_[session_id]->get_socket().is_open() && session_list_[session_id]->get_status() == room)
            {
                session_list_[session_id]->post_send(false, size, master_data_queue_.back().begin());
                session_list_[session_id]->get_opponent_session()->post_send(false, size, master_data_queue_.back().begin());
            }
        }
        break;
    
    // 공지사항
    case chat_server::NOTICE:
        {
            boost::array<BYTE, 1024> send_data;
            CopyMemory(&send_data, packet, size);
            master_data_queue_.push_back(send_data);

            auto iter = connected_session_map_.begin();
            for (iter = connected_session_map_.begin(); iter != connected_session_map_.end(); ++iter)
            {
                if (iter->second->get_socket().is_open())
                    iter->second->post_send(false, size, master_data_queue_.back().begin());
            }
        }
        break;


    // 실행될 일 없음???
    default:
        std::cout << message_header->type << std::endl;
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
        //std::cout << "Client connection successed. session_id: " << session->get_session_id() << std::endl;
        
        session->post_receive();
        io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    }
    else
    {
        std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
        io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    }
}

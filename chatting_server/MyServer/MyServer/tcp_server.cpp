#include "config.h"
#include "log_manager.h"
#include "redis_connector.h"

#include "tcp_server.h"



// ---------- public ----------
tcp_server::tcp_server(boost::asio::io_service& io_service, int server_port, int master_buffer_len)
    :io_service_(io_service),
        acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), server_port)),
        strand_accept_(io_service), strand_close_(io_service), strand_receive_(io_service),  strand_send_(io_service)
{
    is_accepting_ = false;
    master_data_queue_.set_capacity(master_buffer_len);
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
    LOG_INFO << "Server Start";
    
    // There are 8 threads.
    for (int i = 0; i < 8; i++)
        io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
}
 
void tcp_server::close_session(const int session_id)
{
    std::string user_key = session_list_[session_id]->get_user_key();
    std::string user_id = session_list_[session_id]->get_user_id();
    
    if (connected_session_map_.find(user_id) != connected_session_map_.end())
        connected_session_map_.erase(user_id);

    LOG_INFO << "Client connection closed. session_id: " << session_id;
    
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
    case chat_server::VERIFY_REQ:
        {
            chat_server::packet_verify_req verify_message;
            verify_message.ParseFromArray(packet + message_header_size, message_header->size);
        
            std::string redis_key = verify_message.key_string();
            std::string redis_value = verify_message.value_user_id();

            // 레디스 인증 키 일치
            if (redis_connector::get_instance()->get(redis_key) == redis_value)
            {
                session_list_[session_id]->set_user_key(redis_key);
                session_list_[session_id]->set_user_id(redis_value);
                session_list_[session_id]->set_status(lobby);

                connected_session_map_.insert(std::pair<std::string, tcp_session*>(redis_value, session_list_[session_id]));

                LOG_INFO << "Client cookie verified. user_id: " << session_list_[session_id]->get_user_id();
                session_list_[session_id]->post_verify_ans(true);


                // 입장 메세지
                chat_server::packet_chat_normal normal_message;
                normal_message.set_user_id("");
                normal_message.set_chat_message(redis_value + "님이 입장하셨습니다.");

                MESSAGE_HEADER header;

                header.size = verify_message.ByteSize();
                header.type = chat_server::NORMAL;

                boost::array<BYTE, 1024>* send_data = new boost::array<BYTE, 1024>;
                CopyMemory(send_data, (void*)&header, message_header_size);
                master_data_queue_.push_back(*send_data);

                LOG_CHAT << "[chat_system] " << normal_message.user_id() << ": " << normal_message.chat_message();

                for (auto iter = connected_session_map_.begin(); iter != connected_session_map_.end(); ++iter)
                {
                    if (iter->second->get_socket().is_open() && iter->second->get_status() == lobby)
                        iter->second->post_send(false, size, master_data_queue_.back().begin());
                }
                
            }
            // 불일치
            else
            {
                LOG_WARN << "process_packet() - Client cookie does not verified. user_id: " << session_list_[session_id]->get_user_id();
                session_list_[session_id]->post_verify_ans(false);
            }

        }
        break;

    case chat_server::LOGOUT_REQ:
        {
            chat_server::packet_logout_req logout_message;
            logout_message.ParseFromArray(packet + message_header_size, message_header->size);

            std::string user_id = logout_message.user_id();

            // 의미없는 if문
            if (user_id == session_list_[session_id]->get_user_id())
            {
                LOG_INFO << "Client logout successed. user_id: " << session_list_[session_id]->get_user_id();
                session_list_[session_id]->post_logout_ans(true);
            }
            else
            {
                LOG_WARN << "process_packet() - Client logout failed. user_id: " << session_list_[session_id]->get_user_id();
                session_list_[session_id]->post_logout_ans(false);
            }

        }
        break;


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

    case chat_server::LEAVE_MATCH_NTF:
        {
            session_list_[session_id]->set_opponent_session(nullptr);
            session_list_[session_id]->set_status(lobby);
        }
        break;


    case chat_server::NORMAL:
        {
            chat_server::packet_chat_normal normal_message;
            normal_message.ParseFromArray(packet + message_header_size, message_header->size);

            boost::array<BYTE, 1024> send_data;
            CopyMemory(&send_data, packet, size);
            master_data_queue_.push_back(send_data);

            LOG_CHAT << "[chat_normal] " << normal_message.user_id() << ": " << normal_message.chat_message();

            for (auto iter = connected_session_map_.begin(); iter != connected_session_map_.end(); ++iter)
            {
                if (iter->second->get_socket().is_open() && iter->second->get_status() == lobby)
                    iter->second->post_send(false, size, master_data_queue_.back().begin());
            }
        }
        break;

    case chat_server::WHISPER:
        {
            chat_server::packet_chat_whisper whisper_message;
            whisper_message.ParseFromArray(packet + message_header_size, message_header->size);

            boost::array<BYTE, 1024> send_data;
            CopyMemory(&send_data, packet, size);
            master_data_queue_.push_back(send_data);

            auto iter = connected_session_map_.find(whisper_message.target_id());
            if (session_list_[session_id]->get_socket().is_open() && iter != connected_session_map_.end())
            {
                // 본인에게 귓속말을 한 경우
                if (session_list_[session_id]->get_user_id() == iter->second->get_user_id())
                {
                    master_data_queue_.pop_back();

                    session_list_[session_id]->post_whisper_error();
                }
                // 정상적인 경우
                else
                {
                    LOG_CHAT << "[chat_whisper] " << whisper_message.user_id() << "->" << whisper_message.target_id() << ": " << whisper_message.chat_message();

                    session_list_[session_id]->post_send(false, size, master_data_queue_.back().begin());
                    iter->second->post_send(false, size, master_data_queue_.back().begin());
                }
            }
            // 귓속말 상대가 없는 경우
            else
            {
                master_data_queue_.pop_back();

                session_list_[session_id]->post_whisper_error();
            }
        }
        break;

    case chat_server::ROOM:
        {
            chat_server::packet_chat_room room_message;
            room_message.ParseFromArray(packet + message_header_size, message_header->size);

            boost::array<BYTE, 1024> send_data;
            CopyMemory(&send_data, packet, size);
            master_data_queue_.push_back(send_data);

            LOG_CHAT << "[chat_room] " << room_message.user_id() << "->" << session_list_[session_id]->get_opponent_session()->get_user_id() << ": " << room_message.chat_message();

            if (session_list_[session_id]->get_socket().is_open() && session_list_[session_id]->get_status() == room)
            {
                session_list_[session_id]->post_send(false, size, master_data_queue_.back().begin());
                session_list_[session_id]->get_opponent_session()->post_send(false, size, master_data_queue_.back().begin());
            }
        }
        break;
    
    case chat_server::NOTICE:
        {
            chat_server::packet_chat_notice notice_message;
            notice_message.ParseFromArray(packet + message_header_size, message_header->size);

            boost::array<BYTE, 1024> send_data;
            CopyMemory(&send_data, packet, size);
            master_data_queue_.push_back(send_data);

            LOG_CHAT << "[chat_notice] " << notice_message.user_id() << ": " << notice_message.chat_message();

            for (auto iter = connected_session_map_.begin(); iter != connected_session_map_.end(); ++iter)
            {
                if (iter->second->get_socket().is_open())
                    iter->second->post_send(false, size, master_data_queue_.back().begin());
            }
        }
        break;


    // 패킷 타입 불명
    default:
        LOG_WARN << "process_packet() - Message type is unknown. : " << message_header->type;
        break;
    }
}


// ---------- private ----------
void tcp_server::post_accept()
{
   if (session_queue_.empty())
    {
        is_accepting_ = false;
        
        return;
    }

    is_accepting_ = true;
    int session_id = session_queue_.front();

    session_queue_.pop_front();

    acceptor_.async_accept(session_list_[session_id]->get_socket(),
        boost::bind(&tcp_server::handle_accept, this,
            session_list_[session_id],
            boost::asio::placeholders::error)
    );
}

void tcp_server::handle_accept(tcp_session* session, const boost::system::error_code& error)
{
    if (!error)
    {
        LOG_INFO << "Client connection successed. session_id: " << session->get_session_id();
        
        session->post_receive();
        io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    }
    else
    {
        LOG_WARN << "handle_accept() - Error No: " << error.value() << " Error Message: " << error.message();
        io_service_.post(strand_accept_.wrap(boost::bind(&tcp_server::post_accept, this)));
    }
}

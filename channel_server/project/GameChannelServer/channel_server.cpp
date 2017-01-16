#include "channel_server.h"

namespace ch = channel_serv;

tcp_server::tcp_server(boost::asio::io_service & io_service, friends_manager& friends, match_manager& match, packet_handler& packet_handler)
    : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),PORT_NUMBER))
    , friends_manager_(friends)
    , match_manager_(match)
    , packet_handler_(packet_handler)
{
    accepting_flag_ = false;
}

tcp_server::~tcp_server()
{
    for (size_t i = 0; i < session_list_.size(); ++i)
    {
        if (session_list_[i]->get_socket().is_open())
        {
            session_list_[i]->get_socket().close();
        }
        delete session_list_[i];
    }
}

void tcp_server::init(const int n_max_session_count)
{
    for (int i = 0; i < n_max_session_count; ++i)
    {
        session *p_session = new session(i, acceptor_.get_io_service(), this);
        session_list_.push_back(p_session);
        session_queue_.push_back(i);
    }
}

void tcp_server::start()
{
    std::cout << "서버 시작 ...." << std::endl;
    post_accept();
}

void tcp_server::close_session(const int n_session_id)
{
    std::cout << "세션 ID : " << n_session_id << std::endl;
    //std::cout << "종료 클라이언트 토큰 : " << session_list_[n_session_id]->get_token();
    //std::cout << " 클라이언트 ID : " << session_list_[n_session_id]->get_user_id() << std::endl << std::endl;
    
    session_list_[n_session_id]->get_socket().close();
    session_queue_.push_back(n_session_id);

    if (accepting_flag_ == false)
    {
        post_accept();
    }
}

void tcp_server::process_packet(const int n_session_id, const char * p_data)
{
    packet_header *p_header = (packet_header *)p_data;

    switch (p_header->ID)
    {
    case ch::MESSAGE_ID::FRIENDS_REQ:
        {
            friends_manager_.process_friends_function(get_session(n_session_id), &p_data[packet_header_size], p_header->size);
        }
        break;
    case ch::MESSAGE_ID::PLAY_FRIENDS_REQ:
        {
            
        }
        break;
    case ch::MESSAGE_ID::PLAY_RANK_REQ:
        {
            
        }
        break;
    case ch::MESSAGE_ID::JOIN_REQ:
        {
            if(friends_manager_.lobby_login_process(get_session(n_session_id), &p_data[packet_header_size], p_header->size) == false)
            {
                close_session(n_session_id);
            }
           
        }
        break;
    case ch::MESSAGE_ID::LOGOUT_NTF:
        {
            if (friends_manager_.lobby_logout_process(get_session(n_session_id), &p_data[packet_header_size], p_header->size))
            {
                close_session(n_session_id);
            }
        }
        break;
    default:
        // 정의하지 않은 메세지는 로그로 남김
        break;
    }
}

bool tcp_server::post_accept()
{
    if (session_queue_.empty())
    {
        accepting_flag_ = false;
        return false;
    }
    
    accepting_flag_ = true;
    int n_session_id = session_queue_.front();
    
    session_queue_.pop_front();
    
    acceptor_.async_accept(session_list_[n_session_id]->get_socket(), boost::bind(&tcp_server::handle_accept, this, session_list_[n_session_id], boost::asio::placeholders::error));

    return true;
}

void tcp_server::handle_accept(session * p_session, const boost::system::error_code & error)
{
    if (!error)
    {
        std::cout << "클라이언트 접속 성공, SessionID: " << p_session->get_session_id() << std::endl;
        p_session->init();
        p_session->post_receive();
        post_accept();
    }
    else
    {
        std::cout << "error No: " << error.value() << "error Message : " << error.message() << std::endl;
    }
}

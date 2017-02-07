#include "channel_server.h"



tcp_server::tcp_server(boost::asio::io_service & io_service, friends_manager& friends, match_manager& match, packet_handler& packet_handler)
    : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port))
    , friends_manager_(friends)
    , match_manager_(match)
    , packet_handler_(packet_handler)
{
    accepting_flag_ = false;
    load_server_config();
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

void tcp_server::init()
{
    for (int i = 0; i < max_session_count; ++i)
    {
        session *p_session = new session(i, acceptor_.get_io_service(), this);
        session_list_.push_back(p_session);
        session_queue_.push_back(i);
    }
}

void tcp_server::start()
{
    for (int i = 0; i < max_thread; ++i)
    {
        post_accept();
        boost::thread t(boost::bind(&boost::asio::io_service::run, &acceptor_.get_io_service()));
    }
    log_manager::get_instance()->get_logger()->info("[Channel Server] [Thread : {0:d} | Port : {1:d}]", max_thread, port);
}

void tcp_server::close_session(const int n_session_id)
{
    
    session *request_session = session_list_[n_session_id];
        
    if (request_session->get_status() == status::LOGIN)
    {
        //비정상 종료
        friends_manager_.del_redis_token(request_session->get_token());
        friends_manager_.del_id_in_user_map(request_session->get_user_id());
        log_manager::get_instance()->get_logger()->info("[Unusual Terminate] [User ID : { }]", request_session->get_user_id());
    }
    else if (request_session->get_status() == status::LOGOUT)
    {
        //정상 종료
        log_manager::get_instance()->get_logger()->info("[Log Out] [User ID : { }]", request_session->get_user_id());
    }
    else if (request_session->get_status() == status::MATCH_COMPLETE)
    {
        friends_manager_.del_id_in_user_map(request_session->get_user_id());
        log_manager::get_instance()->get_logger()->info("[Log Out] [Matching complete] [User ID : { }]", request_session->get_user_id());
        //정상 종료
    }
    else if (request_session->get_status() == status::MATCH_RECVER)
    {
        friends_manager_.del_redis_token(request_session->get_token());
        friends_manager_.del_id_in_user_map(request_session->get_user_id());
        log_manager::get_instance()->get_logger()->info("[Unusual Terminate] [Match_recver] [User ID : { }]", request_session->get_user_id());
        //비정상 종료
    }
    else if (request_session->get_status() == status::MATCH_REQUEST)
    {
        friends_manager_.del_redis_token(request_session->get_token());
        friends_manager_.del_id_in_user_map(request_session->get_user_id());
        log_manager::get_instance()->get_logger()->info("[Unusual Terminate] [Match_request] [User ID : { }]", request_session->get_user_id());
        //비정상 종료
    }
    else
    {

    }
    
    request_session->set_status(status::WAIT);

    request_session->get_socket().close();
    session_queue_.push_back(n_session_id);

    if (accepting_flag_ == false)
    {
        post_accept();
    }
}

void tcp_server::process_packet(const int n_session_id, const char * p_data)
{
    packet_header *p_header = (packet_header *)p_data;
    session *request_session = get_session(n_session_id);
    switch (p_header->type)
    {
    case message_type::FRIENDS_REQ:
    {
        friends_manager_.process_friends_function(get_session(n_session_id), &p_data[packet_header_size], p_header->size);
        return;
    }
    case message_type::PLAY_FRIENDS_REL:
    {
        match_manager_.process_matching_with_friends(request_session, &p_data[packet_header_size], p_header->size);
        return;
    }
    case message_type::PLAY_RANK_REQ:
    {
        match_manager_.process_matching(request_session, &p_data[packet_header_size], p_header->size);
        return;
    }
    case message_type::JOIN_REQ:
    {
        friends_manager_.lobby_login_process(get_session(n_session_id), &p_data[packet_header_size], p_header->size);
        return;
    }
    case message_type::LOGOUT_REQ:
    {
        friends_manager_.lobby_logout_process(get_session(n_session_id), &p_data[packet_header_size], p_header->size);
        close_session(n_session_id);
        return;
    }
    case message_type::MATCH_CONFIRM:
    {
        session *request_session = get_session(n_session_id);
        if (request_session->get_status() == status::MATCH_COMPLETE)
        {
            close_session(n_session_id);
        }
        else
        {
            error_report error_message;
            error_message.set_error_string("Your did not request match!");
            int send_data_size = error_message.ByteSize() + packet_header_size;
            char *send_data = packet_handler_.incode_message(error_message);
            
            request_session->post_send(false, send_data_size, send_data);
        }
        return;
    }
    default:
        // 정의하지 않은 메세지는 로그로 남김
        break;
    }
}

void tcp_server::rematching_request(session * request_session)
{
    match_manager_.rematching_que(request_session);
}

bool tcp_server::post_accept()
{
    session_queue_mtx.lock();
   
    if (session_queue_.empty())
    {
        accepting_flag_ = false;
        return false;
    }
    accepting_flag_ = true;
    int n_session_id = session_queue_.front();
    session_queue_.pop_front();
    
    session_queue_mtx.unlock();
    
    acceptor_.async_accept(session_list_[n_session_id]->get_socket(), boost::bind(&tcp_server::handle_accept, this, session_list_[n_session_id], boost::asio::placeholders::error));

    return true;
}

void tcp_server::handle_accept(session * p_session, const boost::system::error_code & error)
{
    if (!error)
    {
        p_session->init();
        p_session->post_receive();
        log_manager::get_instance()->get_logger()->info("[Session Connect] [Session ID : {0:d}]",p_session->get_session_id());
        post_accept();
    }
    else
    {
        log_manager::get_instance()->get_logger()->warn("[Accept Error No : {0:d}] [Error Message : { }] ",error.value(), error.message());
    }
}

void tcp_server::load_server_config()
{
    config::get_instance()->get_value("SERVER_CONFIG", "MAX_THREAD", max_thread);
    config::get_instance()->get_value("SERVER_CONFIG", "PORT", port);
    config::get_instance()->get_value("SERVER_CONFIG", "MAX_BUFFER_LEN", max_buffer_len);
    config::get_instance()->get_value("SERVER_CONFIG", "MAX_TOKEN_SIZE", max_token_size);
    config::get_instance()->get_value("SERVER_CONFIG", "MAX_SESSION_COUNT", max_session_count);
}
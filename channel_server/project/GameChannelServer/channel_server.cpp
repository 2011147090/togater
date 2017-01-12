#include "channel_server.h"

namespace ch = channel_serv;
PacketHandler h; // 메시지 확인용 클래스 추후 삭제 & 로그로 변환

tcp_server::tcp_server(boost::asio::io_service & io_service, redispp::Connection& conn)
    : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),PORT_NUMBER))
    , conn_(conn)
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
            prcoess_friends_packet(n_session_id, &p_data[packet_header_size], p_header->size); // 미완
        }
        break;
    case ch::MESSAGE_ID::PLAY_FRIENDS_REQ:
        {
            process_play_friends_packet(n_session_id, &p_data[packet_header_size], p_header->size); // 미완
        }
        break;
    case ch::MESSAGE_ID::PLAY_RANK_REQ:
        {
            process_play_rank_packet(n_session_id, &p_data[packet_header_size], p_header->size); // 미완 
        }
        break;
    case ch::MESSAGE_ID::JOIN_REQ:
        {
           process_join_packet(n_session_id, &p_data[packet_header_size], p_header->size); // 완료
        }
        break;
    case ch::MESSAGE_ID::LOGOUT_NTF:
        {
            process_logout_packet(n_session_id, &p_data[packet_header_size], p_header->size); // 완료
        }
        break;
    default:
        // 정의하지 않은 메세지는 로그로 남김
        break;
    }
}

void tcp_server::process_join_packet(const int n_session_id, const char * p_data, const int parse_size)
{
    ch::join_req packet;
    packet.ParseFromArray(p_data, parse_size);
    
    auto user_id = this->conn_.get(packet.token());
    if (user_id.result().is_initialized() != false)
    {
        /*db 쿼리 받아오기*/
        int rating = 0;
        int battle_history = 100;
        int win = 100;
        int lose = 0;
        ///////////////////////

        session_list_[n_session_id]->set_token(packet.token().c_str());
        session_list_[n_session_id]->set_user_info(rating, user_id.result().value().c_str());
        
        ch::join_ans ans;
        ch::user_info *user = ans.mutable_my_info();
        user->set_battle_history(battle_history);
        user->set_rating(h.check_rating(rating));
        user->set_win(win);
        user->set_lose(lose);
        user->set_user_id(session_list_[n_session_id]->get_user_id());
        
        char *ans_buffer = new char[ans.ByteSize() + packet_header_size + 1];
        packet_header *header = (packet_header *)ans_buffer;
        header->ID = ch::MESSAGE_ID::JOIN_ANS;
        header->size = ans.ByteSize();
        ans.SerializeToArray(&ans_buffer[packet_header_size], ans.ByteSize());

        session_list_[n_session_id]->post_send(false, ans.ByteSize() + packet_header_size, ans_buffer);
    }
    else
    {
        std::cout << "Token is not valid" << std::endl;
        close_session(n_session_id);
    }
}

void tcp_server::process_play_rank_packet(const int n_session_id, const char * p_data, const int parse_size)
{
    ch::play_rank_game_req packet;
    packet.ParseFromArray(p_data, parse_size);
    h.Handle(packet);
}

void tcp_server::process_play_friends_packet(const int n_session_id, const char * p_data, const int parse_size)
{
    ch::play_friends_game_req packet;
    packet.ParseFromArray(p_data, parse_size);
    h.Handle(packet);
}

void tcp_server::prcoess_friends_packet(const int n_session_id, const char * p_data, const int parse_size)
{
    ch::friends_req packet;
    packet.ParseFromArray(p_data, parse_size);
    
}

void tcp_server::process_logout_packet(const int n_session_id, const char * p_data, const int parse_size)
{
    ch::logout_ntf packet;
    packet.ParseFromArray(p_data, parse_size);
    std::string del_token = packet.token();
    this->conn_.del(del_token);

    std::cout << "user logout !" << std::endl;
    close_session(n_session_id);
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

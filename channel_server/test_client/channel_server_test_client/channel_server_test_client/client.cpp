#include "client.h"
#include "protocol.h"


client::client(boost::asio::io_service & io_service, packet_handler& handler)
    : io_service_(io_service)
    , socket_(io_service)
    , seq_number(0)
    , token_flag(true)
    , packet_handler_(handler)
{

}

client::~client()
{
}

void client::client_connect(boost::asio::ip::tcp::endpoint &endpoint)
{
    endpoint_ = endpoint;
    socket_.async_connect(endpoint, boost::bind(&client::connect_handle, this, boost::asio::placeholders::error));
}


void client::post_join_message(const char *token)
{
    channel_serv::join_req message;
    message.set_token(token);
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_logout_message(const char * token)
{
    channel_serv::logout_ntf message;
    message.set_token(token);
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_play_message()
{
    channel_serv::play_rank_game_req message;
    channel_serv::user_info *my_info = message.mutable_my_info();
    my_info->set_battle_history(get_battle_history());
    my_info->set_win(get_win());
    my_info->set_lose(get_lose());
    my_info->set_rating(packet_handler_.check_rating(get_rating()));
    my_info->set_user_id(get_user_id());
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_play_with_friends_message(const char *user_id)
{
    channel_serv::play_friends_game_req message;
    message.set_flag_id(channel_serv::play_friends_game_req::APPLY);
    message.set_recv_id(user_id);
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_search_message(const char *user_id)
{
    channel_serv::friends_req message;
    message.set_req(channel_serv::friends_req::SEARCH);
    message.set_user_id(user_id);
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_add_message(const char *user_id)
{
    channel_serv::friends_req message;
    message.set_req(channel_serv::friends_req::ADD);
    message.set_user_id(user_id);
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_del_message(const char *user_id)
{
    channel_serv::friends_req message;
    message.set_req(channel_serv::friends_req::DEL);
    message.set_user_id(user_id);
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_accept_message()
{
    channel_serv::play_friends_game_req message;
    message.set_flag_id(channel_serv::play_friends_game_req::ACCEPT);
    message.set_recv_id(target_id);
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_deny_message()
{
    channel_serv::play_friends_game_req message;
    message.set_flag_id(channel_serv::play_friends_game_req::DENY);
    message.set_recv_id(target_id);
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_send(const bool que_flag, char * send_message, int n_size)
{
    if(que_flag == false)
        send_data_que.push_back(send_message);
    
    if (que_flag == false && send_data_que.size() > 1)
    {
        return;
    }

    boost::asio::async_write(socket_, boost::asio::buffer(send_message, n_size),
        boost::bind(&client::send_handle, this, boost::asio::placeholders::error, 
            boost::asio::placeholders::bytes_transferred));
}

void client::recv_handle(const boost::system::error_code & error, size_t bytes_transferred)
{
    if (error)
    {
        if (error == boost::asio::error::eof)
        {
            if (!token_flag)
            {
                std::cout << "토큰 인증 실패" << std::endl;
            }
            else
            {
                std::cout << "서버와 연결이 끊어졌습니다" << std::endl;
            }
        }
        else
        {
            if(token_flag)
                std::cout << "error No:" << error.value() << "error Message" << std::endl;
        }
    }
    else
    {
        process_packet(trans_buffer.data(), bytes_transferred);
        post_recv();
    }
}

void client::send_handle(const boost::system::error_code & error, size_t bytes_transferred)
{
    delete[] send_data_que.front();
    send_data_que.pop_front();
    if (send_data_que.empty() == false)
    {
        char *data = send_data_que.front();
        packet_header *header = (packet_header *)data;
        post_send(true, data, header->size + packet_header_size);
    }
}

void client::post_recv()
{
    memset(&trans_buffer, '\0', sizeof(trans_buffer));
    socket_.async_read_some(boost::asio::buffer(trans_buffer),
        boost::bind(&client::recv_handle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void client::connect_handle(const boost::system::error_code &error)
{
    if (error)
    {
        client_connect(endpoint_);
    }
    else
    {
        std::cout << "접속 하였습니다" << std::endl;
        post_recv();
    }
}

void client::process_packet(char * data, int nsize)
{
    packet_header *header = (packet_header *)data;
    switch (header->ID)
    {
    case channel_serv::MESSAGE_ID::JOIN_ANS:
        process_join(&data[packet_header_size],header->size);
        break;
    case channel_serv::MESSAGE_ID::FRIENDS_ANS:
        process_friends(&data[packet_header_size], header->size);
        break;
    case channel_serv::MESSAGE_ID::MATCH_COMPLETE:
        process_game(&data[packet_header_size], header->size);
        break;
    case channel_serv::MESSAGE_ID::PLAY_FRIENDS_REQ:
        process_game_with_friends(&data[packet_header_size], header->size);
        break;
    case channel_serv::MESSAGE_ID::ERROR_MSG:
        process_error(&data[packet_header_size], header->size);
        break;
    default:
        break;
    }
}

void client::process_friends(char * data, int nsize)
{
    channel_serv::friends_ans ans;
    ans.ParseFromArray(data, nsize);
    std::cout << "\n\n친구 찾음 : " << (ans.mutable_user_information())->user_id() << std::endl;
}

void client::process_join(char * data, int nsize)
{
    channel_serv::join_ans ans;
    channel_serv::user_info *info;
    ans = packet_handler_.decode_join_ans_message(data, nsize);
    info = ans.mutable_my_info();
    if (ans.friends_id_list_size() > 0)
    {

    }
    battle_history = info->battle_history();
    win = info->win();
    lose = info->lose();
    rating = info->rating();
    user_id = info->user_id();
    std::cout << " 내 아이디 : " << user_id << std::endl;
    token_flag = true;
}

void client::process_game(char * data, int nsize)
{
    channel_serv::matching_complete_ans ans;
    channel_serv::user_info *op_player;
    ans.ParseFromArray(data, nsize);
    op_player = ans.mutable_opponent_player();
    std::cout << "\n\n 방정보 : " << ans.room_number();
    std::cout << "\n상대방 아이디 : "<<op_player->user_id()<<"\n";
}

void client::process_game_with_friends(char * data, int size)
{
    channel_serv::play_friends_game_req ans;
    ans.ParseFromArray(data, size);
    target_id.clear();
    target_id = ans.recv_id();
    switch (ans.flag_id())
    {
    case channel_serv::play_friends_game_req::APPLY:
        std::cout<<ans.recv_id() << "가 게임신청하심" << std::endl;
        return;
    case channel_serv::play_friends_game_req::DENY:
        std::cout << ans.recv_id() << "가 거절하심" << std::endl;
        return;
    }
}

void client::process_error(char * data, int nsize)
{
    channel_serv::error_msg ans;
    ans.ParseFromArray(data, nsize);
    std::cout << "error : " << ans.error_message() << "\n";
}


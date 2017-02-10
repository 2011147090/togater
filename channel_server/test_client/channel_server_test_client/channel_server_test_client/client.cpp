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
    join_request message;
    message.set_token(token);
    message.set_id("kain");
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_logout_message(const char * token)
{
    logout_request message;
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_play_message()
{
    match_request message;
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_play_with_friends_message(const char *user_id)
{
    match_with_friends_relay message;
    message.set_type(match_with_friends_relay::APPLY);
    message.set_target_id(user_id);
    
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_search_message(const char *user_id)
{
    friends_request message;
    message.set_type(friends_request::SEARCH);
    message.mutable_target_info()->set_id(user_id);
    
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_add_message(const char *user_id)
{
    friends_request message;
    message.set_type(friends_request::ADD);
    message.mutable_target_info()->set_id(user_id);
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_del_message(const char *user_id)
{
    friends_request message;
    message.set_type(friends_request::DEL);
    message.mutable_target_info()->set_id(user_id);
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_accept_message()
{
    match_with_friends_relay message;
    message.set_type(match_with_friends_relay::ACCEPT);
    post_send(false, packet_handler_.incode_message(message), message.ByteSize() + packet_header_size);
}

void client::post_deny_message()
{
    match_with_friends_relay message;
    message.set_type(match_with_friends_relay::DENY);
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
    switch (header->type)
    {
    case message_type::JOIN_ANS:
        process_join(&data[packet_header_size],header->size);
        break;
    case message_type::FRIENDS_ANS:
        process_friends(&data[packet_header_size], header->size);
        break;
    case message_type::MATCH_COMPLETE:
        process_game(&data[packet_header_size], header->size);
        std::cout << "매치됨" << std::endl;
        break;
    case message_type::PLAY_FRIENDS_REL:
        process_game_with_friends(&data[packet_header_size], header->size);
        break;
    case message_type::ERROR_MSG:
        process_error(&data[packet_header_size], header->size);
        break;
    default:
        break;
    }
}

void client::process_friends(char * data, int nsize)
{
    friends_response  message;
    message.ParseFromArray(data, nsize);
    switch (message.type())
    {
        case friends_response::ADD_SUCCESS:
        {
            std::cout<<"ADD SUCCESS"<<std::endl;
            return;
        }
        case friends_response::ADD_FAIL:
        {
            std::cout << "ADD FAIL" << std::endl;
            return;
        }
        case friends_response::DEL_SUCCESS:
        {
            std::cout << "DEL SUCCESS" << std::endl;
            return;
        }
        case friends_response::DEL_FAIL:
        {
            std::cout << "DEL FAIL" << std::endl;
            return;
        }
        case friends_response::SEARCH_SUCCESS:
        {
            std::cout << "SEARCH SUCCESS" << std::endl;
            return;
        }
        case friends_response::SEARCH_FAIL:
        {
            std::cout << "SEARCH FAIL" << std::endl;
            return;
        }
    }
}

void client::process_join(char * data, int nsize)
{
    join_response message;
    message.ParseFromArray(data, nsize);
    game_history *history = message.mutable_history();
    std::vector<std::string> list;
    
    
    
    win = history->win();
    lose = history->lose();
    rating = history->rating_score();
    std::cout << "win : " << win << " lost : " << lose << " rating score : " << rating << std::endl;
    std::cout << "my friends" << std::endl;
    for (int i = 0; i < message.friends_list_size(); i++)
    {
        const basic_info &p = message.friends_list(i);
        std::cout << p.id() << std::endl;
    }
    token_flag = true;
}

void client::process_game(char * data, int nsize)
{
    socket_.close();
}

void client::process_game_with_friends(char * data, int size)
{
}

void client::process_error(char * data, int nsize)
{
}


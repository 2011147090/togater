#include "client.h"
#include "protocol.h"

using namespace channel_serv;

client::client(boost::asio::io_service & io_service)
    : io_service_(io_service)
    , socket_(io_service)
    , seq_number(0)
    , token_flag(true)
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
    packet_header header;
    join_req pay_load;
    pay_load.set_token(token);

    header.ID = MESSAGE_ID::JOIN_REQ;
    header.size = pay_load.ByteSize();    
    char *send_msg_buffer = new char[header.size + packet_header_size];
    memcpy(send_msg_buffer, &header, packet_header_size);
    
    pay_load.SerializeToArray(&send_msg_buffer[packet_header_size], header.size);
    post_send(false, send_msg_buffer,header.size + packet_header_size);
    token_flag = false;
}

void client::post_logout_message(const char * token)
{
    logout_ntf pay_load;
    pay_load.set_token(token);

    char *send_msg_buffer = new char[packet_header_size + pay_load.ByteSize()];
    packet_header *header = (packet_header *)send_msg_buffer;
    header->ID = MESSAGE_ID::LOGOUT_NTF;
    header->size = pay_load.ByteSize();

    pay_load.SerializeToArray(&send_msg_buffer[packet_header_size], header->size);
    post_send(false, send_msg_buffer, header->size + packet_header_size);
}

void client::post_play_message()
{

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
        post_join_message("200");
    }
}

void client::process_packet(char * data, int nsize)
{
    packet_header *header = (packet_header *)data;
    switch (header->ID)
    {
    case MESSAGE_ID::JOIN_ANS:
        process_join(&data[packet_header_size],header->size);
        post_logout_message("200");
        break;
    case MESSAGE_ID::FRIENDS_ANS:
        process_friends(&data[packet_header_size], header->size);
        break;
    case MESSAGE_ID::MATCH_COMPLETE:
        process_game(&data[packet_header_size], header->size);
        break;
    case MESSAGE_ID::PLAY_FRIENDS_REQ:
        process_game_with_friends(&data[packet_header_size], header->size);
        break;
    case MESSAGE_ID::ERROR_MSG:
        process_error(&data[packet_header_size], header->size);
        break;
    default:
        break;
    }
}

void client::process_friends(char * data, int nsize)
{
    friends_ans ans;
    ans.ParseFromArray(data, nsize);
}

void client::process_join(char * data, int nsize)
{
    join_ans ans;
    ans.ParseFromArray(data, nsize);
    if (ans.friends_id_list_size() > 0)
    {

    }
    cout << "My ID : " << ans.my_info().user_id() << endl;
    cout << "My battle history : " << ans.my_info().battle_history() << endl;
    cout << "Win : " << ans.my_info().win() << " Lose : " << ans.my_info().lose() << endl;
    token_flag = true;
}

void client::process_game(char * data, int nsize)
{
    matching_complete_ans ans;
    ans.ParseFromArray(data, nsize);
}

void client::process_game_with_friends(char * data, int size)
{
    play_friends_game_req ans;
    ans.ParseFromArray(data, size);
}

void client::process_error(char * data, int nsize)
{
    error_msg ans;
    ans.ParseFromArray(data, nsize);
}

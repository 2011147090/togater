#include "network_manager.h"

network_manager::network_manager()
{
}

network_manager::~network_manager() {}

bool network_manager::init_singleton()
{
    thread_sync sync;

    is_connected_ = false;
    
    socket_ = new tcp::socket(io_service_);

    return true;
}

bool network_manager::release_singleton()
{
    thread_sync sync;

    is_connected_ = false;

    work_thread_->join();

    if (socket_ != NULL)
        delete socket_;

    return true;
}

void network_manager::connect(std::string ip, std::string port)
{
    thread_sync sync;
    
    work_thread_ = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));

    tcp::resolver resolver(socket_->get_io_service());
    tcp::resolver::query query(tcp::v4(), ip, port);

    auto endPointIter = resolver.resolve(query);
    tcp::resolver::iterator end;

    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endPointIter != end)
    {
        socket_->close();
        socket_->connect(*endPointIter++, error);
    }

    if (error)
        throw boost::system::system_error(error);

    is_connected_ = true;
    
    recv_thread_ = new boost::thread(boost::bind(&network_manager::handle_read, this));

    io_service_.run();
}

void network_manager::disconnect()
{
    thread_sync sync;

    socket_->close();
}

bool network_manager::is_run()
{
    thread_sync sync;

    return is_connected_;
}

bool network_manager::is_socket_open()
{
    thread_sync sync;
    
    if (!socket_->is_open() && is_connected_)
    {
        is_connected_ = false;
        return false;
    }

    return true;
}

void network_manager::handle_read()
{
    while (true)
    {
        if (!is_socket_open())
            break;

        socket_->receive(boost::asio::buffer(recv_buf_));

        thread_sync sync;

        MESSAGE_HEADER message_header;

        CopyMemory(&message_header, recv_buf_.begin(), message_header_size);
        
        switch (message_header.type)
        {
        case logic_server::ENTER_ANS:
        {
            logic_server::packet_enter_ans message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_enter_ans(message);
        }
        break;

        case logic_server::PROCESS_TURN_REQ:
        {
            logic_server::packet_process_turn_req message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_process_turn_req(message);
        }
        break;

        case logic_server::PROCESS_TURN_NTF:
        {
            logic_server::packet_process_turn_ntf message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_process_turn_ntf(message);

            break;
        }

        case logic_server::GAME_STATE_NTF:
        {
            logic_server::packet_game_state_ntf message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_game_state_ntf(message);
        }
        break;

        case logic_server::PROCESS_CHECK_CARD_NTF:
        {
            logic_server::packet_process_check_card_ntf message;

            if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                break;

            process_packet_process_check_card_ntf(message);
        }
        break;

        }
    }
}

void network_manager::handle_send(logic_server::message_type msg_type, const protobuf::Message& message)
{
    thread_sync sync;

    MESSAGE_HEADER header;
    header.size = message.ByteSize();
    header.type = msg_type;

    int buf_size = 0;
    buf_size = message_header_size + message.ByteSize();

    CopyMemory(send_buf_.begin(), (void*)&header, message_header_size);

    message.SerializeToArray(send_buf_.begin() + message_header_size, header.size);

    socket_->write_some(boost::asio::buffer(send_buf_));
}

void network_manager::handle_write()
{
    //thread_sync sync;

    //int player_key = 1;
    //std::string room_key = "temp";
    //int money;

    //wait_turn = false;

    //logic_server::packet_enter_req enter_req_packet;
    //enter_req_packet.set_room_key(room_key);
    //enter_req_packet.set_player_key(player_key);

    //this->handle_send(logic_server::ENTER_REQ, enter_req_packet);

    //while (is_socket_open())
    //{
    //    while (!wait_turn) {}

    //    do {
    //        std::cout << "enter batting money key (give up - input 0) : " << std::endl;
    //        std::cin >> money;

    //    } while (total_money_ < money);

    //    batting_money_ += money;

    //    logic_server::packet_process_turn_ans process_turn_packet;
    //    process_turn_packet.set_money(money);
    //    process_turn_packet.set_player_key(player_key);

    //    wait_turn = false;
    //    //system("cls");
    //    this->handle_send(logic_server::PROCESS_TURN_ANS, process_turn_packet);
    //}
}


/*protobuf::io::ArrayInputStream input_array_stream(recv_buf_.c_array(), recv_buf_.size());
protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

MESSAGE_HEADER message_header;

input_coded_stream.ReadRaw(&message_header, message_header_size);

const void* payload_ptr = NULL;
int remainSize = 0;
input_coded_stream.GetDirectBufferPointer(&payload_ptr, &remainSize);

protobuf::io::ArrayInputStream payload_array_stream(payload_ptr, message_header.size);
protobuf::io::CodedInputStream payload_input_stream(&payload_array_stream);

input_coded_stream.Skip(message_header.size);*/
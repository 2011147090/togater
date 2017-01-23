#ifndef __NETWORK_MANAGER_H__
#define __NETWORK_MANAGER_H__

// boost
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// google network_manager buffer
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "logic_server.pb.h"

#include "my_thread_sync.h"
#include "singleton.h"

using namespace google;

const std::string LOGIC_SERVER_IP("127.0.0.1");

//"192.168.1.201");

using boost::asio::ip::tcp;

#define network_mgr network_manager::get_instance()

enum { BUFSIZE = 128 };

typedef struct _MESSAGE_HEADER {
    protobuf::uint32 size;
    logic_server::message_type type;
} MESSAGE_HEADER;

const int message_header_size = sizeof(MESSAGE_HEADER);

class network_manager : public singleton<network_manager>, public multi_thread_sync<network_manager> {
private:
    tcp::socket* socket_;
    
    bool is_connected_;
    int test_count_;

    std::string room_key_;

    boost::array<BYTE, BUFSIZE> recv_buf_;
    boost::array<BYTE, BUFSIZE> send_buf_;

    boost::thread* work_thread_;
    boost::thread* recv_thread_;
    boost::asio::io_service io_service_;

public:
    network_manager();
    ~network_manager();

    virtual bool init_singleton();
    virtual bool release_singleton();

    bool is_run();

    void connect(std::string ip, std::string port);
    void disconnect();

    bool is_socket_open();

    void handle_read();
    void handle_write();
    
    void send_packet_enter_req(std::string room_key, std::string player_key);
    void send_packet_process_turn_ans(int money);
    void send_packet_disconnect_room_ntf();

private:
    void handle_send(logic_server::message_type msg_type, const protobuf::Message& message);
    
    void process_packet_enter_ans(logic_server::packet_enter_ans packet);
    void process_packet_process_turn_req(logic_server::packet_process_turn_req packet);
    void process_packet_process_turn_ntf(logic_server::packet_process_turn_ntf packet);
    void process_packet_process_check_card_ntf(logic_server::packet_process_check_card_ntf packet);
    void process_packet_game_state_ntf(logic_server::packet_game_state_ntf packet);
};

#endif
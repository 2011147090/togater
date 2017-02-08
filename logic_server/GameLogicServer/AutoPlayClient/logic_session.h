#pragma once

#include "network_session.h"
#include "logic_server.pb.h"

const std::string LOGIC_SERVER_IP("192.168.1.24");
const int LOGIC_SERVER_PORT(8600);

class logic_session : public network_session {
private:
    typedef struct _MESSAGE_HEADER {
        protobuf::uint32 size;
        logic_server::message_type type;
    } MESSAGE_HEADER;

    const int message_header_size = sizeof(MESSAGE_HEADER);

    void handle_send(logic_server::message_type msg_type, const protobuf::Message& message, bool must_recv);

    void process_packet_enter_ans(logic_server::packet_enter_ans packet);
    void process_packet_process_turn_req(logic_server::packet_process_turn_req packet);
    void process_packet_process_turn_ntf(logic_server::packet_process_turn_ntf packet);
    void process_packet_process_check_card_ntf(logic_server::packet_process_check_card_ntf packet);
    void process_packet_game_state_ntf(logic_server::packet_game_state_ntf packet);

public:
    void send_packet_enter_req(std::string room_key, std::string player_key);
    void send_packet_process_turn_ans(int money);
    void send_packet_disconnect_room_ntf();
    void send_packet_game_state_ntf();
};
#pragma once
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/text_format.h>
#include <packet.pb.h>
#define MAX_RECEIVE_BUFFER_LEN 256
#define TOKEN_SIZE 5

using namespace std;
using namespace google;

const unsigned short PORT_NUMBER = 31400;
const unsigned short MAX_SESSION_COUNT = 5000;

struct packet_header
{
    protobuf::uint32 size;
    channel_serv::MESSAGE_ID ID;
};

const int packet_header_size = sizeof(packet_header);

class PacketHandler
{
public:
    void Handle(const channel_serv::friends_req& message) const
    {
        PrintMessage(message);
    }
    void Handle(const channel_serv::friends_ans& message) const
    {
        PrintMessage(message);
    }
    void Handle(const channel_serv::play_friends_game_req& message) const
    {
        PrintMessage(message);
    }
    void Handle(const channel_serv::play_rank_game_req& message) const
    {
        PrintMessage(message);
    }
    void Handle(const channel_serv::join_req& message) const
    {
        PrintMessage(message);
    }
    void Handle(const channel_serv::join_ans& message) const
    {
        PrintMessage(message);
    }
    void Handle(const channel_serv::matching_complete_ans& message) const
    {
        PrintMessage(message);
    }
    void Handle(const channel_serv::error_msg& message) const
    {
        PrintMessage(message);
    }
    void Handle(const channel_serv::logout_ntf& message) const
    {
        PrintMessage(message);
    }
protected:
    void PrintMessage(const protobuf::Message& message) const
    {
        string textFormatStr;
        protobuf::TextFormat::PrintToString(message, &textFormatStr);
        printf("%s\n", textFormatStr.c_str());
    }
};
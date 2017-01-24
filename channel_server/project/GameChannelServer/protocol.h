#pragma once
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/text_format.h>
#include <packet.pb.h>

using namespace std;
using namespace google;

/* redis */
#define REDIS_SERVER_IP "127.0.0.1"
#define REDIS_PORT "6379"
#define REDIS_PWD "password"

/* server */
#define MAX_RECEIVE_BUFFER_LEN 256
#define TOKEN_SIZE 32
const unsigned short PORT_NUMBER = 31400;
const unsigned short MAX_SESSION_COUNT = 5000;

/* session status */

typedef enum session_status  
{
    WAIT
    , LOGIN
    , LOGOUT
    , MATCH_REQUEST
    , MATCH_RECVER
    , MATCH_COMPLETE
} status;

struct packet_header
{
    protobuf::uint32 size;
    channel_serv::MESSAGE_ID ID;
};

const int packet_header_size = sizeof(packet_header);

class packet_handler
{
public:
    char* incode_message(const channel_serv::friends_req& message)
    {
        return incoding(channel_serv::MESSAGE_ID::FRIENDS_REQ, message);
    }

    char* incode_message(const channel_serv::friends_ans& message)
    {
        return incoding(channel_serv::MESSAGE_ID::FRIENDS_ANS, message);
    }

    char* incode_message(const channel_serv::play_friends_game_req& message)
    {
        return incoding(channel_serv::MESSAGE_ID::PLAY_FRIENDS_REQ, message);
    }

    char* incode_message(const channel_serv::play_rank_game_req& message)
    {
        return incoding(channel_serv::MESSAGE_ID::PLAY_RANK_REQ, message);
    }

    char* incode_message(const channel_serv::join_req& message)
    {
        return incoding(channel_serv::MESSAGE_ID::JOIN_REQ, message);
    }

    char* incode_message(const channel_serv::join_ans& message)
    {
        return incoding(channel_serv::MESSAGE_ID::JOIN_ANS, message);
    }

    char* incode_message(const channel_serv::matching_complete_ans& message)
    {
        return incoding(channel_serv::MESSAGE_ID::MATCH_COMPLETE, message);
    }

    char* incode_message(const channel_serv::error_msg& message)
    {
        return incoding(channel_serv::MESSAGE_ID::ERROR_MSG, message);
    }

    char* incode_message(const channel_serv::logout_ntf& message)
    {
        return incoding(channel_serv::MESSAGE_ID::LOGOUT_NTF, message);
    }

    inline void decode_message(channel_serv::join_req &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(channel_serv::join_ans &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(channel_serv::logout_ntf &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(channel_serv::error_msg &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(channel_serv::matching_complete_ans &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(channel_serv::play_friends_game_req &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(channel_serv::play_rank_game_req &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(channel_serv::friends_req &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(channel_serv::friends_ans &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }
    
    channel_serv::RATING check_rating(const int rating)
    {
        if (rating < 100) return channel_serv::RATING::BRONZE;
        else if (rating < 200) return channel_serv::RATING::SILVER;
        else if (rating < 300) return channel_serv::RATING::GOLD;
        else if (rating < 400) return channel_serv::RATING::PLATINUM;
        else if (rating < 500) return channel_serv::RATING::DIAMOND;
        else if (rating < 800) return channel_serv::RATING::MASTER;
        else return channel_serv::RATING::CHAL;
    }
private:
    char *incoding(const channel_serv::MESSAGE_ID id, const protobuf::Message& message)
    {
        char *incoding_data = new char[packet_header_size + message.ByteSize()];
        packet_header *header = (packet_header *)incoding_data;

        header->ID = id;
        header->size = message.ByteSize();
        message.SerializePartialToArray(&incoding_data[packet_header_size], header->size);

        return incoding_data;
    }

    void PrintMessage(const protobuf::Message& message) const
    {
        string textFormatStr;
        protobuf::TextFormat::PrintToString(message, &textFormatStr);
        printf("%s\n", textFormatStr.c_str());
    }
};
#pragma once
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/text_format.h>
#include "channel_protobuf.pb.h"

using namespace std;
using namespace google;
using namespace channel_server;
/* redis */
#define REDIS_SERVER_IP "192.168.1.201"
#define REDIS_PORT "6379"
#define REDIS_PWD "password"

/* server */
#define MAX_RECEIVE_BUFFER_LEN 256
#define TOKEN_SIZE 32
const unsigned short PORT_NUMBER = 8800;
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
/* protobuf struct */
typedef rating rating_name;
/* packet about join */
typedef packet_join_req join_request;
typedef packet_join_ans join_response;

/* packet about friends */
typedef packet_friends_req friends_request;
typedef packet_friends_ans friends_response;

typedef packet_friends_req_req_type friends_request_type;
const unsigned char friends_add = packet_friends_req::ADD;
const unsigned char friends_del = packet_friends_req::DEL;
const unsigned char friends_search = packet_friends_req::SEARCH;

/* packet about game */
typedef packet_play_rank_game_req match_request;
typedef packet_play_rank_game_ans match_response;
typedef packet_matching_complete_ans match_complete;
typedef packet_play_friends_game_rel match_with_friends_relay;
const unsigned char normal_game_apply = packet_play_friends_game_rel::APPLY;
const unsigned char normal_game_accept = packet_play_friends_game_rel::ACCEPT;
const unsigned char normal_game_deny = packet_play_friends_game_rel::DENY;

/* packet about logout*/
typedef packet_logout_req logout_request;
typedef packet_logout_ans logout_response;

/* packet about error */
typedef packet_error_message error_report;

struct packet_header
{
    protobuf::uint32 size;
    message_type type;
};

const int packet_header_size = sizeof(packet_header);

class packet_handler
{
public:
    char* incode_message(const friends_request& message)
    {
        return incoding(message_type::FRIENDS_REQ, message);
    }

    char* incode_message(const friends_response& message)
    {
        return incoding(message_type::FRIENDS_ANS, message);
    }

    char* incode_message(const match_with_friends_relay& message)
    {
        return incoding(message_type::PLAY_FRIENDS_REL, message);
    }

    char* incode_message(const match_request& message)
    {
        return incoding(message_type::PLAY_RANK_REQ, message);
    }

    char* incode_message(const match_response& message)
    {
        return incoding(message_type::PLAY_RANK_ANS, message);
    }

    char* incode_message(const join_request& message)
    {
        return incoding(message_type::JOIN_REQ, message);
    }

    char* incode_message(const join_response& message)
    {
        return incoding(message_type::JOIN_ANS, message);
    }

    char* incode_message(const match_complete& message)
    {
        return incoding(message_type::MATCH_COMPLETE, message);
    }

    char* incode_message(const error_report& message)
    {
        return incoding(message_type::ERROR_MSG, message);
    }

    char* incode_message(const logout_request& message)
    {
        return incoding(message_type::LOGOUT_REQ, message);
    }

    char* incode_message(const logout_response& message)
    {
        return incoding(message_type::LOGOUT_ANS, message);
    }

    inline void decode_message(join_request &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(join_response &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(logout_request &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(logout_response &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(error_report &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(match_complete &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(match_with_friends_relay &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(match_request &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(match_response &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(friends_request &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    inline void decode_message(friends_response &message, const char *decoding_data, const int data_size)
    {
        message.ParseFromArray(decoding_data, data_size);
    }

    rating check_rating(const int rating)
    {
        if (rating < 100) return rating::bronze;
        else if (rating < 200) return rating::silver;
        else if (rating < 300) return rating::gold;
        else if (rating < 400) return rating::platinum;
        else if (rating < 500) return rating::diamond;
        else if (rating < 800) return rating::master;
        else return rating::challenger;
    }
private:
    char *incoding(const message_type type, const protobuf::Message& message)
    {
        char *incoding_data = new char[packet_header_size + message.ByteSize()];
        packet_header *header = (packet_header *)incoding_data;

        header->type = type;
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
#pragma once

#include "chat_protobuf.pb.h"

using namespace google;


const int PORT_NUMBER = 8700;
const int MAX_RECEIVE_BUFFER_LEN = 1024;
const int MAX_MASTER_BUFFER_LEN = 1024;

typedef struct _MESSAGE_HEADER {
    protobuf::uint32 size;
    chat_server::message_type type;
} MESSAGE_HEADER;

const int message_header_size = sizeof(MESSAGE_HEADER);
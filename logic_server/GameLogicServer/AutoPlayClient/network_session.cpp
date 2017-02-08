#include "pre_header.h"
#include "network_session.h"
#include "logger.h"

network_session::network_session()
{
    is_connected_ = false;
}

network_session::~network_session() {}

bool network_session::connect(std::string ip, int port)
{
    thread_sync sync;

    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socket_ == SOCKET_ERROR) {
        logger::print("Socket Error!");
        return false;
    }

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (::connect(socket_, (SOCKADDR*)&addr, sizeof(addr)) == -1) {
        logger::print("Connent error!!");
        return false;
    }

    is_connected_ = true;

    return true;
}

bool network_session::disconnect()
{
    thread_sync sync;

    is_connected_ = false;
    closesocket(socket_);

    return true;
}
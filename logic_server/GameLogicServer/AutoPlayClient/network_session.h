#pragma once

#include "pre_header.h"
#include "my_thread_sync.h"

#define network_mgr network_manager::get_instance()

enum { BUFSIZE = 512 };

class network_session : public multi_thread_sync<network_session> {
protected:
    SOCKET socket_;

    bool is_connected_;

public:
    network_session();
    ~network_session();

    bool connect(std::string ip, int port);
    bool disconnect();
};
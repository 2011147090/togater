#ifndef __NETWORK_MANAGER_H__
#define __NETWORK_MANAGER_H__

#include "logic_server.pb.h"
#include "chat_protobuf.pb.h"

// boost
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// google network_manager buffer
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include "my_thread_sync.h"
#include "singleton.h"

#include "network_session.h"

using boost::asio::ip::tcp;

#define network_mgr network_manager::get_instance()

class network_manager : public singleton<network_manager>, public multi_thread_sync<network_manager> {
private:
    std::string room_key_;
    std::string player_key_;
    std::string player_id_;

    network_session* session[2];
    
public:
    enum SESSION_TYPE { CHAT_SESSION = 0, LOGIC_SESSION };
    
    network_session* get_session(SESSION_TYPE session_type);

    virtual bool init_singleton();
    virtual bool release_singleton();

    std::string get_player_key();
    std::string get_player_id();
    std::string get_room_key();

    void set_player_key(std::string key);
    void set_player_id(std::string id);
    void set_room_key(std::string key);

    network_manager();
    ~network_manager();
};

#endif
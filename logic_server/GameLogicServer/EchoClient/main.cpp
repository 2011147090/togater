// google protocol buffer
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "logic_server.pb.h"

// Boost
#include <boost/asio.hpp>
#include <boost/pool/pool.hpp>
#include <boost/function.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/config.hpp>
#include <boost/unordered_map.hpp>

#include <iostream>

using namespace boost::asio;
using namespace google;

typedef struct _MESSAGE_HEDER {
    protobuf::int32 size;
    logic_server::message_type type;
} MESSAGE_HEADER;

const int message_header_size = sizeof MESSAGE_HEADER;

class echo_client {
public:
    std::thread* session;

    echo_client(int i)
    {
        std::cout << i << " : session" << std::endl;

        session = new std::thread(&echo_client::run, this);
    }

    void end()
    {
        session->join();

        delete session;
    }
    
    void run()
    {
        boost::asio::io_service io_service;

        ip::tcp::resolver resolver(io_service);
        ip::tcp::resolver::query query(ip::tcp::v4(), "192.168.1.201", "8600"); //201

        auto endpoint_iter = resolver.resolve(query);
        ip::tcp::resolver::iterator end;

        boost::system::error_code error = boost::asio::error::host_not_found;

        ip::tcp::socket socket(io_service);

        socket.connect(*endpoint_iter++, error);
        

        if (error)
        {
            std::cout << "end session" << std::endl;
            return;
        }

        io_service.run();

        logic_server::packet_echo_ntf packet;
        packet.set_str("hello");

        MESSAGE_HEADER header;
        header.size = packet.ByteSize();
        header.type = logic_server::ECHO_NTF;


        for (;;)
        {
            boost::array<char, 128> buf;
            boost::system::error_code error;

            CopyMemory(buf.begin(), (void*)&header, message_header_size);
            
            packet.SerializeToArray(buf.begin() + message_header_size, header.size);

            std::cout << packet.str() << std::endl;
            
            socket.write_some(boost::asio::buffer(buf.begin(), message_header_size + header.size), error);
            
            if (error)
            {
                std::cout << "end session" << std::endl;
                break;
            }

            socket.read_some(boost::asio::buffer(buf.begin(), message_header_size + header.size), error);

            if (error)
            {
                std::cout << "end session" << std::endl;
                break;
            }

            Sleep(1000);
        }
    }
};

void main()
{
    echo_client* session[100];

    for (int i = 0; i < 100; i++)
        session[i] = new echo_client(i);

    getchar();

   // for (int i = 0; i < 50; i++)
        //session[i]->end();
}
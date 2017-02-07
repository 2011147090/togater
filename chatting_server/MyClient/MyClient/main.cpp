#include "tcp_client.h"


int main(int argc, char* argv[])
{
    boost::asio::io_service io_service;

    auto endpoint = boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address::from_string("192.168.1.8"), PORT_NUMBER);

    tcp_client chat_client(io_service);
        
    // --------------------------
    std::string id = argv[1];
    chat_client.set_id(id);
    chat_client.set_key(id);
    // --------------------------

    chat_client.connect(endpoint);

    boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));

    chat_client.post_verify_req();

    int count = 0;
    while (chat_client.is_login())
    {
        //std::string message;
        //std::getline(std::cin, message);

        Sleep(1000);
        //chat_client.post_normal("Hi!");
    }

    io_service.stop();
    thread.join();
    chat_client.close();

    std::cout << "클라이언트를 종료해 주세요" << std::endl;

    return 0;
}
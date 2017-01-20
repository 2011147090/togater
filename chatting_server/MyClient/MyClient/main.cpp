#include "tcp_client.h"


int main()
{
    boost::asio::io_service io_service;

    auto endpoint = boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address::from_string("127.0.0.1"), PORT_NUMBER);

    tcp_client chat_client(io_service);
    // -----temporary-----
    std::cout << "이름? (user1, user2, user3)" << std::endl;
    std::cin >> chat_client.user_id_;
    if (chat_client.user_id_ == "user1")
        chat_client.key_ = "qwerty";
    else if (chat_client.user_id_ == "user2")
        chat_client.key_ = "asdfgh";
    else if (chat_client.user_id_ == "user3")
        chat_client.key_ = "zxcvbn";
    else
        return 0;
    // -----temporary-----
    chat_client.connect(endpoint);
    
    boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
    
    chat_client.post_verify();

    std::string message;
    while (std::getline(std::cin, message))
    {
        chat_client.post_send(false, message);
    }
    
    io_service.stop();

    chat_client.close();

    thread.join();

    std::cout << "클라이언트를 종료해 주세요" << std::endl;

    return 0;
}
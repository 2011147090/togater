#include "tcp_client.h"


int main()
{
    boost::asio::io_service io_service;

    auto endpoint = boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address::from_string("192.168.1.8"), PORT_NUMBER);

    tcp_client chat_client(io_service);
    

    // -----temporary start-----
    std::string id;
    std::cout << "이름? (user1 ~ user3)" << std::endl;
    std::cin >> id;
    
    chat_client.set_id(id);
    if (id == "user1")
        chat_client.set_key("qwerty");
    else if (id == "user2")
        chat_client.set_key("asdfgh");
    else if (id == "user3")
        chat_client.set_key("zxcvbn");
    else
        chat_client.set_key("no_key");
    // ------temporary end------
    

    chat_client.connect(endpoint);
    
    boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
    
    chat_client.post_verify_req();
    

    while (chat_client.is_login())
    {
        std::string message;
        std::getline(std::cin, message);

        chat_client.post_normal(message);
    }
    
    io_service.stop();
    thread.join();
    chat_client.close();


    std::cout << "클라이언트를 종료해 주세요" << std::endl;

    return 0;
}
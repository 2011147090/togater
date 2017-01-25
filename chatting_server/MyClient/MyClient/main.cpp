#include "tcp_client.h"


int main()
{
    boost::asio::io_service io_service;

    auto endpoint = boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address::from_string("192.168.1.8"), PORT_NUMBER);

    tcp_client chat_client(io_service);
    

    // -----temporary start-----
    std::string id;
    std::cout << "이름? (user1 ~ user12)" << std::endl;
    std::cin >> id;
    
    chat_client.set_id(id);
    if (id == "user1")
        chat_client.set_key("qwerty");
    else if (id == "user2")
        chat_client.set_key("asdfgh");
    else if (id == "user3")
        chat_client.set_key("zxcvbn");
    else if (id == "user4")
        chat_client.set_key("wertyu");
    else if (id == "user5")
        chat_client.set_key("sdfghj");
    else if (id == "user6")
        chat_client.set_key("xcvbnm");
    else if (id == "user7")
        chat_client.set_key("ertyui");
    else if (id == "user8")
        chat_client.set_key("dfghjk");
    else if (id == "user9")
        chat_client.set_key("cvbnm,");
    else if (id == "user10")
        chat_client.set_key("rtyuio");
    else if (id == "user11")
        chat_client.set_key("fghjkl");
    else if (id == "user12")
        chat_client.set_key("vbnm,.");
    else if (id == "qwer")
        chat_client.set_key("mG2n9Z_y7K7ik_4ZXKpwoPzP-p6SAlKW");
    else
        return 0;
    // ------temporary end------
    

    chat_client.connect(endpoint);
    
    boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
    

    // -----temporary start-----
    chat_client.post_verify_req();
    // ------temporary end------


    std::string message;
    while (std::getline(std::cin, message))
    {
        chat_client.post_normal(message);
    }
    
    io_service.stop();

    chat_client.close();

    thread.join();

    std::cout << "클라이언트를 종료해 주세요" << std::endl;

    return 0;
}
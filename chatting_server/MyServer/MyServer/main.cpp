#include "tcp_server.h"


const int MAX_SESSION_COUNT = 1000;

int main()
{
    boost::asio::io_service io_service;

    tcp_server server(io_service);
    server.init(MAX_SESSION_COUNT);
    server.start();

    io_service.run();


    std::cout << "��Ʈ��ũ ���� ����" << std::endl;

    return 0;
}
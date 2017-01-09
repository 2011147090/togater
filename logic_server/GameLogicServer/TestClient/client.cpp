#include "preHeader.h"
#include "connect_session.h"

void main()
{
	try
	{
		boost::asio::io_service io_service;

		connect_session proto(io_service);

		boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
		proto.connect();

		boost::thread send(boost::bind(&connect_session::handle_write, &proto));
		boost::thread recv(boost::bind(&connect_session::handle_read, &proto));

		io_service.run();

		while (proto.is_run())
		{
		}

		recv.join();
		send.join();

		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	int in;
	std::cout << "END";
	std::cin >> in;

	return;
}
#include "preHeaders.h"
#include "tcp_server.h"
#include "log_manager.h"
#include "redis_manager.h"
#include "logic_worker.h"

int main(int argc, char* argv[])
{
	try
	{
		log->info("server_start");

		if (!log_manager::get_instance()->init_singleton())
		{
			log->error("failed_init_log_manager");

			return 0;
		}

		if (!redis_manager::get_instance()->init_singleton())
		{
			log->error("failed_init_redis_manager");

			return 0;
		}

		if (!logic_worker::get_instance()->init_singleton())
		{
			log->error("failed_init_logic_worker");

			return 0;
		}

		boost::asio::io_service io_Service;
		
		tcp_server tcp_server(io_Service);
				
		boost::thread_group io_thread;
		io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &io_Service));
		io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &io_Service));
		io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &io_Service));
		io_thread.create_thread(boost::bind(&boost::asio::io_service::run, &io_Service));

		io_thread.join_all();

		getchar();
	}
	catch (std::exception& e)
	{
		log->error("{}", e.what());
		std::cerr << e.what() << std::endl;
	}

	log->error("server_close");

	return 0;
}
#include "log_manager.h"

log_manager::log_manager() {}

log_manager::~log_manager() {}

bool log_manager::init_singleton()
{
	return true;
}

std::shared_ptr<spd::logger> log_manager::get_logger()
{	
	std::shared_ptr<spd::logger> system_log = spd::daily_logger_mt("logic_server", "system_log", 0, 0);
	spd::drop_all();

	return system_log;
}
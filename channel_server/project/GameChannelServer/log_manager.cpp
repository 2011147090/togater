#include "log_manager.h"



log_manager::log_manager()
{
    logger = spd::stdout_color_mt("console");
}


log_manager::~log_manager()
{
}

std::string log_manager::get_log_mode()
{
    return std::string();
}

void log_manager::set_log_mode()
{

}

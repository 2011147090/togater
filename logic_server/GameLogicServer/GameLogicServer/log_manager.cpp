#include "pre_headers.h"
#include "log_manager.h"

log_manager::log_manager() {}

log_manager::~log_manager() {}

bool log_manager::init_singleton()
{
    is_debug_mode_ = false;
    console = spd::stdout_color_mt("console");

    return true;
}

bool log_manager::release_singleton()
{
    thread_sync sync;

    system_log->info("release_log_manager");

    spd::drop_all();

    for (auto iter = logger_list.begin(); iter != logger_list.end();)
    {
        (*iter).second.logger.reset();

        iter = logger_list.erase(iter);
    }

    return true;
}

void log_manager::set_create_time(int hour, int min)
{

}

std::shared_ptr<spd::logger> log_manager::get_logger(std::string logger_name, std::string file_name)
{
    thread_sync sync;

    if (is_debug_mode_)
    {
        return console;
    }

    auto iter = logger_list.find(file_name);

    if (iter == logger_list.end())
    {
        LOGGER_INFO new_logger;
        new_logger.logger = spd::daily_logger_mt(logger_name, file_name, 0, 0);
        new_logger.write_loop_num = 10;
        new_logger.cur_log_num = 1;

        spd::drop_all();
        
        logger_list.insert(std::pair<std::string, LOGGER_INFO>(file_name, new_logger));
        
        return (*logger_list.find(file_name)).second.logger;
    }
        
    if ((*iter).second.cur_log_num >= (*iter).second.write_loop_num)
    {
        spd::drop_all();

        (*iter).second.logger.reset();
        (*iter).second.cur_log_num = 0;
        (*iter).second.logger = spd::daily_logger_mt(logger_name, file_name, 0, 0);
    }
    else
        (*iter).second.cur_log_num++;

    return (*iter).second.logger;
}

bool log_manager::set_logger(std::string name, int write_loop_num)
{
    thread_sync sync;

    auto iter = logger_list.find(name);

    if (iter == logger_list.end())
        return false;

    (*iter).second.write_loop_num = write_loop_num;

    return true;
}

bool log_manager::erase_logger(std::string name)
{
    thread_sync sync;

    auto iter = logger_list.find(name);

    if (iter == logger_list.end())
        return false;

    spd::drop(name);

    (*iter).second.logger.reset();
    logger_list.erase(iter);

    return true;
}

void log_manager::set_debug_mode(bool is_debug_mode)
{
    is_debug_mode_ = is_debug_mode;
}
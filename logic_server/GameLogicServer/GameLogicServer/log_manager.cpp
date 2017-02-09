#include "pre_headers.h"
#include "log_manager.h"

log_manager::log_manager() {}

log_manager::~log_manager() {}

std::string _itoa(int i)
{
    char temp[10] = "";
    itoa(i, temp, 10);

    return temp;
}

bool log_manager::init_singleton()
{
    is_debug_mode_ = false;
    console_ = spd::stdout_color_mt("console");

    time(&cur_time_);

    SYSTEMTIME local_time;
    GetLocalTime(&local_time);
    
    time_str_ = "_" + _itoa(local_time.wYear);
    time_str_ += "-" + _itoa(local_time.wMonth);
    time_str_ += "-" + _itoa(local_time.wDay);
    time_str_ += "-" + _itoa(local_time.wHour);
    time_str_ += "-" + _itoa(local_time.wMinute) + ".txt";;
    
    return true;
}

bool log_manager::release_singleton()
{
    thread_sync sync;

    system_log->info("release_log_manager");

    spd::drop_all();

    for (auto iter = logger_list_.begin(); iter != logger_list_.end();)
    {
        (*iter).second.logger.reset();

        iter = logger_list_.erase(iter);
    }

    return true;
}

std::string log_manager::check_daily_time()
{
    SYSTEMTIME local_time;

    time_t temp_time;
    time(&temp_time);

    if (cur_time_ >= temp_time + 3600)
    {
        GetLocalTime(&local_time);
        cur_time_ = temp_time;
    
        time_str_ = "_" + _itoa(local_time.wYear);
        time_str_ += "-" + _itoa(local_time.wMonth);
        time_str_ += "-" + _itoa(local_time.wDay);
        time_str_ += "-" + _itoa(local_time.wHour);
        time_str_ += "-" + _itoa(local_time.wMinute) + ".txt";
    }
    
    return time_str_;
}

std::shared_ptr<spd::logger> log_manager::get_logger(std::string logger_name, std::string file_name)
{
    thread_sync sync;

    if (is_debug_mode_)
    {
        return console_;
    }

    auto iter = logger_list_.find(file_name);

    if (iter == logger_list_.end())
    {
        LOGGER_INFO new_logger;
        new_logger.logger = spd::basic_logger_mt(logger_name, file_name + check_daily_time());
        new_logger.write_loop_num = 10;
        new_logger.cur_log_num = 1;

        spd::drop_all();
        
        logger_list_.insert(std::pair<std::string, LOGGER_INFO>(file_name, new_logger));
        
        return (*logger_list_.find(file_name)).second.logger;
    }
        
    if ((*iter).second.cur_log_num >= (*iter).second.write_loop_num)
    {
        spd::drop_all();

        (*iter).second.logger.reset();
        (*iter).second.cur_log_num = 0;
        (*iter).second.logger = spd::basic_logger_mt(logger_name, file_name + check_daily_time());
    }
    else
        (*iter).second.cur_log_num++;

    return (*iter).second.logger;
}

bool log_manager::set_logger(std::string name, int write_loop_num)
{
    thread_sync sync;

    auto iter = logger_list_.find(name);

    if (iter == logger_list_.end())
        return false;

    (*iter).second.write_loop_num = write_loop_num;

    return true;
}

bool log_manager::erase_logger(std::string name)
{
    thread_sync sync;

    auto iter = logger_list_.find(name);

    if (iter == logger_list_.end())
        return false;

    spd::drop(name);

    (*iter).second.logger.reset();
    logger_list_.erase(iter);

    return true;
}

void log_manager::set_debug_mode(bool is_debug_mode)
{
    is_debug_mode_ = is_debug_mode;
}
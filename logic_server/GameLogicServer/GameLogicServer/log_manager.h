#pragma once
#include "pre_headers.h"
#include "singleton.h"
#include "critical_section.h"

#define log_mgr log_manager::get_instance()

typedef struct _LOGGER_INFO {
    std::shared_ptr<spd::logger> logger;
    int write_loop_num;
    int cur_log_num;
} LOGGER_INFO;

class log_manager : public singleton<log_manager>, public multi_thread_sync<log_manager> {
private:
    std::map<std::string, LOGGER_INFO> logger_list_;
    std::shared_ptr<spdlog::logger> console_;
    bool is_debug_mode_;

    std::string time_str_;
    time_t cur_time_;

public:
    std::shared_ptr<spd::logger> get_logger(std::string logger_name, std::string file_name);
    
    std::string check_daily_time();

    bool set_logger(std::string name, int write_loop_num);
    bool erase_logger(std::string name);
    
    virtual bool init_singleton();
    virtual bool release_singleton();

    void set_debug_mode(bool is_debug_mode);

    virtual ~log_manager();
    log_manager();
};

#define system_log log_mgr->get_logger("logic_server", "system_log")
#define log(name) log_mgr->get_logger("match_", name)
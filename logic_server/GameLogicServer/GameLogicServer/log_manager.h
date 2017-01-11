#pragma once
#include "preHeaders.h"
#include "singleton.h"
#include "critical_section.h"

typedef struct _LOGGER_INFO {
    std::shared_ptr<spd::logger> logger;
    int write_loop_num;
    int cur_log_num;
} LOGGER_INFO;

class log_manager : public singleton<log_manager>, public multi_thread_sync<log_manager> {
private:
    std::map<std::string, LOGGER_INFO> logger_list;

public:
    std::shared_ptr<spd::logger> get_logger(std::string logger_name, std::string file_name);
    
    bool set_logger(std::string name, int write_loop_num);
    bool erase_logger(std::string name);
    
    virtual bool init_singleton();

    ~log_manager();
    log_manager();
};

#define system_log log_manager::get_instance()->get_logger("logic_server", "system_log")
#define log(name) log_manager::get_instance()->get_logger("match_", name)
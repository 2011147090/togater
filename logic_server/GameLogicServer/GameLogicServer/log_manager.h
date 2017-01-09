#pragma once
#include "preHeaders.h"
#include "singleton.h"

class log_manager : public singleton<log_manager> {
public:
	std::shared_ptr<spd::logger> get_logger();
	
	virtual bool init_singleton();

	~log_manager();
	log_manager();
};

#define log log_manager::get_instance()->get_logger()
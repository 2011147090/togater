#pragma once
#include "preHeaders.h"
#include "singleton.h"

class redis_manager : public singleton<redis_manager> {
private:
	redispp::Connection* conn;
	CRITICAL_SECTION critical_section;

public:
	virtual bool init_singleton();

	redis_manager();
	~redis_manager();
	
	bool check_room(int room_key);
	void remove_room_info(int room_key);
};
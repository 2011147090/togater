#include "redis_manager.h"

redis_manager::redis_manager()
{
	InitializeCriticalSection(&critical_section);
}

redis_manager::~redis_manager()
{
	LeaveCriticalSection(&critical_section);
}

bool redis_manager::init_singleton()
{
	EnterCriticalSection(&critical_section);

	conn = new redispp::Connection("127.0.0.1", "6379", "password", false);

	if (conn == NULL)
	{
		LeaveCriticalSection(&critical_section);
		return false;
	}

	// test code 
	char str[11] = "";
	itoa(0, str, 10);

	conn->set(str, "0");

	LeaveCriticalSection(&critical_section);

	return true;
}

bool redis_manager::check_room(int room_key)
{
	EnterCriticalSection(&critical_section);

	char room_str[11] = "";
	itoa(room_key, room_str, 10);

	auto value = conn->get(room_str);

	if (!value.result().is_initialized())
	{
		LeaveCriticalSection(&critical_section);

		return false;
	}

	int i = atoi(value.result().get().c_str());
	i++;

	char value_str[11] = "";

	itoa(i, value_str, 10);

	conn->set(room_str, value_str);

	LeaveCriticalSection(&critical_section);

	return true;
}

void redis_manager::remove_room_info(int room_key)
{
	EnterCriticalSection(&critical_section);

	char room_str[11] = "";
	itoa(room_key, room_str, 10);

	conn->del(room_str);

	LeaveCriticalSection(&critical_section);
}
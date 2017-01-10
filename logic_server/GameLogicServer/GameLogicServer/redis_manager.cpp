#include "redis_manager.h"

redis_manager::redis_manager()
{}

redis_manager::~redis_manager()
{}

bool redis_manager::init_singleton()
{
	thread_sync sync;

	conn = new redispp::Connection("127.0.0.1", "6379", "password", false);

	if (conn == NULL)
		return false;

	// test code 
	char str[11] = "";
	itoa(0, str, 10);

	conn->set(str, "0");

	return true;
}

bool redis_manager::check_room(int room_key)
{
	thread_sync sync;

	char room_str[11] = "";
	itoa(room_key, room_str, 10);

	auto value = conn->get(room_str);

	if (!value.result().is_initialized())
		return false;

	int i = atoi(value.result().get().c_str());
	i++;

	char value_str[11] = "";

	itoa(i, value_str, 10);

	conn->set(room_str, value_str);

	return true;
}

void redis_manager::remove_room_info(int room_key)
{
	thread_sync sync;

	char room_str[11] = "";
	itoa(room_key, room_str, 10);

	conn->del(room_str);
}
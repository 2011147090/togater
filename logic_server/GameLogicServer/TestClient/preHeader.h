#pragma once

#include <ctime>
#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Windows.h>

const std::string LOGIC_SERVER_IP("192.168.1.24");

using boost::asio::ip::tcp;

enum { BUFSIZE = 128 };

#include "json_spirit.h"
#include "packet_generator.h"

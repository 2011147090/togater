#pragma once

// Standard
#include <ctime>
#include <string>
#include <iostream>
#include <map>
#include <queue>
#include <random>

// Boost
#include <boost/asio.hpp>
#include <boost/pool/pool.hpp>
#include <boost/function.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/config.hpp>
//#include <boost/random/random_device.hpp>
//#include <boost/random/uniform_int_distribution.hpp>

// Json Spirit
#include <json_spirit.h>

// Speed Log
#include <spdlog\spdlog.h>

// redispp
#include "redispp.h"

// google protocol buffer
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include "logic_server.pb.h"

enum { BUFSIZE = 128 };

using boost::asio::ip::tcp;

namespace spd = spdlog;
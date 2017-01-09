#pragma once

// boost
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// google connect_session buffer
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "logic_server.pb.h"

// standard library
#include <ctime>
#include <iostream>
#include <string>
#include <Windows.h>


using namespace google;

const std::string LOGIC_SERVER_IP("192.168.1.24");

using boost::asio::ip::tcp;

enum { BUFSIZE = 128 };

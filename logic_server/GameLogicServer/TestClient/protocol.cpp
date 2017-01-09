#include "preHeader.h"
#include "protocol.h"
#include "packet_generator.h"

protocol::protocol(boost::asio::io_service& io) : socket_(io)
{
	is_connected_ = false;
	test_count_ = 0;
}

protocol::~protocol() {}

void protocol::connect()
{
	tcp::resolver resolver(socket_.get_io_service());
	tcp::resolver::query query(LOGIC_SERVER_IP, "daytime");

	auto endPointIter = resolver.resolve(query);
	tcp::resolver::iterator end;

	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error && endPointIter != end)
	{
		socket_.close();
		socket_.connect(*endPointIter++, error);
	}

	if (error)
		throw boost::system::system_error(error);

	is_connected_ = true;

	//handle_write();
}

bool protocol::is_run() { return is_connected_; }

bool protocol::is_socket_open()
{
	if (!socket_.is_open() && is_connected_)
	{
		is_connected_ = false;
		return false;
	}

	return true;
}

void protocol::handle_read()
{
	while (is_connected_)
	{
		if (!is_socket_open())
			break;

		//socket_.async_read_some(boost::asio::buffer(recv_buf_), boost::bind(&protocol::handle_read, this));
		socket_.receive(boost::asio::buffer(recv_buf_));

		auto buf_json = packet_generator::packet_to_json(recv_buf_.c_array());

		json_spirit::Object root_obj = buf_json.get_obj();

		if (root_obj[0].value_.get_int() != 0)
		{
			// ³ë ¹öÁ¯
			return;
		}

		switch (root_obj[1].value_.get_int()) {
		case PACKET_PROTOCOL::ENTER_ANS:
			std::cout << "get packet : PACKET_PROTOCOL::ENTER_ANS" << std::endl;
			break;
		}
	}
}

void protocol::handle_write()
{
	if (is_socket_open())
	{
		try {
			json_spirit::Object input_baseobj;
			input_baseobj.push_back(json_spirit::Pair("paket_version", 0));
			input_baseobj.push_back(json_spirit::Pair("protocol_type", 100));
			input_baseobj.push_back(json_spirit::Pair("key", 0));
			input_baseobj.push_back(json_spirit::Pair("room_key", 0));

			json_spirit::Value data(input_baseobj);

			int length = 0;

			BYTE* buf = packet_generator::json_to_packet(data, &length);

			boost::system::error_code error;
			boost::asio::write(socket_, boost::asio::buffer(buf, length), error);

			delete buf;
		}
		catch (std::exception& e)
		{
			is_connected_ = false;
			std::cerr << e.what() << std::endl;
		}

		std::cout << "send packet : PACKET_PROTOCOL::ENTER_REQ" << std::endl;

		//socket_.async_read_some(boost::asio::buffer(recv_buf_), boost::bind(&protocol::handle_read, this));
	}
}
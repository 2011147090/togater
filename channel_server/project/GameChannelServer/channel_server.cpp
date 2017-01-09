#include "channel_server.h"





server::server(boost::asio::io_service & io_service)
	: m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),PORT_NUMBER))
{
	m_bls_accepting = false;
}

server::~server()
{
	for (size_t i = 0; i < m_session_list.size(); ++i)
	{
		if (m_session_list[i]->get_socket().is_open())
		{
			m_session_list[i]->get_socket().close();
		}
		delete m_session_list[i];
	}
}

void server::init(const int n_max_session_count)
{
	for (int i = 0; i < n_max_session_count; ++i)
	{
		session *p_session = new session(i, m_acceptor.get_io_service(), this);
		m_session_list.push_back(p_session);
		m_session_queue.push_back(i);
	}
}

void server::start()
{
	std::cout << "서버 시작 ...." << std::endl;
	post_accept();
}

void server::close_session(const int n_session_id)
{
	std::cout << "클라이언트 접속 종료. 세션 ID: " << n_session_id << std::endl;
	std::cout << "클라이언트 ID : " << m_session_list[n_session_id]->get_token() << std::endl;
	m_session_list[n_session_id]->get_socket().close();
	m_session_queue.push_back(n_session_id);
	if (m_bls_accepting == false)
	{
		post_accept();
	}
}

void server::process_packet(const int n_session_id, const char * p_data)
{
	packet_header *p_header = (packet_header *)p_data;
	PacketHandler h;
	namespace ch = channel_server;
	
	switch (p_header->ID)
	{
	case ch::MESSAGE_ID::FREINDS_REQ:
		{
			ch::freinds_req packet;
			packet.ParseFromArray(&p_data[packet_header_size], p_header->size - packet_header_size);
			h.Handle(packet);
		}
		break;
	case ch::MESSAGE_ID::PLAY_REQ:
		{
			ch::play_req packet;
			packet.ParseFromArray(&p_data[packet_header_size], p_header->size - packet_header_size);
			h.Handle(packet);
		}
		break;
	case ch::MESSAGE_ID::JOIN_NTF:
		{
			ch::join_ntf packet;
			packet.ParseFromArray(&p_data[packet_header_size], p_header->size - packet_header_size);
			h.Handle(packet);
		}
		break;
	case ch::MESSAGE_ID::LOGOUT_NTF:
		{
			ch::logout_ntf packet;
			packet.ParseFromArray(&p_data[packet_header_size], p_header->size - packet_header_size);
			h.Handle(packet);
		}
		break;
	default:
		break;
	}
}

bool server::post_accept()
{
	if (m_session_queue.empty())
	{
		m_bls_accepting = false;
		return false;
	}
	
	m_bls_accepting = true;
	int n_session_id = m_session_queue.front();
	
	m_session_queue.pop_front();
	
	m_acceptor.async_accept(m_session_list[n_session_id]->get_socket(), boost::bind(&server::handle_accept, this, m_session_list[n_session_id], boost::asio::placeholders::error));

	return true;
}

void server::handle_accept(session * p_session, const boost::system::error_code & error)
{
	if (!error)
	{
		std::cout << "클라이언트 접속 성공, SessionID: " << p_session->get_session_id() << std::endl;
		p_session->init();
		p_session->post_receive();
		post_accept();
	}
	else
	{
		std::cout << "error No: " << error.value() << "error Message : " << error.message() << std::endl;
	}
}

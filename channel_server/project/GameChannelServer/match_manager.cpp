#include "match_manager.h"



match_manager::match_manager(packet_handler & packet_handler, friends_manager & friends_manager, redispp::Connection & redis_connection)
    : packet_handler_(packet_handler)
    , friends_manager_(friends_manager)
    , redis_connection_(redis_connection)
{ }

match_manager::~match_manager()
{ }

void match_manager::set_matching_que(session * p_session)
{
    channel_serv::RATING rating = packet_handler_.check_rating(p_session->get_rating());
    switch (rating)
    {
    case channel_serv::RATING::BRONZE:
        bronze_que.push_back(p_session);
        get_matching_que(bronze_que);
        break;
    case channel_serv::RATING::SILVER:
        silver_que.push_back(p_session);
        get_matching_que(silver_que);
        break;
    case channel_serv::RATING::GOLD:
        gold_que.push_back(p_session);
        get_matching_que(gold_que);
        break;
    case channel_serv::RATING::PLATINUM:
        platinum_que.push_back(p_session);
        get_matching_que(platinum_que);
        break;
    case channel_serv::RATING::DIAMOND:
        diamond_que.push_back(p_session);
        get_matching_que(diamond_que);
        break;
    case channel_serv::RATING::MASTER:
        master_que.push_back(p_session);
        get_matching_que(master_que);
        break;
    case channel_serv::RATING::CHAL:
        challenger_que.push_back(p_session);
        get_matching_que(challenger_que);
        break;
    default:
        break;
    }
}

void match_manager::get_matching_que(std::deque<session *> &target_que)
{
    if (target_que.size() > 1)
    {
        //큐배분
    }
    else
    {
        //비동기 타이머 셋팅
    }
}

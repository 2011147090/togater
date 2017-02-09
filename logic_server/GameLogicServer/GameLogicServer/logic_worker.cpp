#include "pre_headers.h"
#include "logic_worker.h"
#include "random_generator.h"
#include "log_manager.h"
#include "database_connector.h"

_PLAYER_INFO::_PLAYER_INFO()
{
    submit_money_= 0;
    sum_money_ = 0;
    remain_money_ = 20;
    submit_card_ = false;
}

_ROOM_INFO::_ROOM_INFO(PLAYER_INFO player_info)
{
    ready_player_num_ = 0;
    turn_count_ = 0;
    hide_card_ = true;
    state_ = GAME_STATE::READY;

    player_[0] = player_info;
}

void _ROOM_INFO::generate_card_queue()
{
    while (!card_list_.empty())
        card_list_.pop();

    std::deque<int> d;
    
    for (int i = 0; i < 4; i++)
        for (int j = 1; j <= 10; j++)
            d.push_back(j);

    srand((unsigned int)time(nullptr));
    std::random_shuffle(d.begin(), d.end());
    card_list_ = std::queue<int>(d);
}

int _ROOM_INFO::get_card()
{
    if (card_list_.empty())
        generate_card_queue();

    int num = card_list_.front();
    card_list_.pop();

    return num;
}

logic_worker::logic_worker() : logic_thread_(nullptr)
{}

logic_worker::~logic_worker() 
{}

bool logic_worker::init_singleton()
{
    thread_sync sync;

    if (logic_thread_ == nullptr)
        logic_thread_ = new boost::thread(&logic_worker::process_queue, this);

    end_server_ = true;

    room_hashs_.reserve(size_t(1000));

    return true;
}

bool logic_worker::release_singleton()
{
    end_server_ = false;

    logic_thread_->join();

    return true;
}

bool logic_worker::create_room(connected_session* session, std::string room_key)
{
    thread_sync sync;

    if (!redis_connector::get_instance()->check_room(room_key))
        return false;
    
    PLAYER_INFO player_info;
    player_info.session_ = session;
    player_info.remain_money_ = 20;
    player_info.sum_money_ = 1;
    player_info.submit_money_ = 0;

    room_hashs_.emplace(std::pair<std::string, ROOM_INFO>(room_key, ROOM_INFO(player_info)));
    return true;
}

bool logic_worker::enter_room_player(connected_session* session, std::string room_key)
{
    thread_sync sync;

    auto iter = room_hashs_.find(room_key);
    
    if (iter != room_hashs_.end())
    {
        PLAYER_INFO player_info;
        player_info.remain_money_ = 20;
        player_info.submit_money_ = 0;
        player_info.sum_money_ = 1;
        player_info.session_ = session;

        iter->second.player_[1] = player_info;

        return true;
    }

    if (!create_room(session, room_key))
        return false;

    return true;
}

bool logic_worker::process_turn(std::string room_key, std::string player_key, int money)
{
    thread_sync sync;

    auto iter = room_hashs_.find(room_key);

    if (iter != room_hashs_.end())
    {
        if (iter->second.player_[0].session_->get_player_key() == player_key)
        {
            iter->second.player_[0].submit_card_ = true;
            iter->second.player_[0].submit_money_ = money;

            return true;
        }
        else if (iter->second.player_[1].session_->get_player_key() == player_key)
        {
            iter->second.player_[1].submit_card_ = true;
            iter->second.player_[1].submit_money_ = money;

            return true;
        }
    }

    return false;
}

bool comp(int const& a, int const& b) {
    if (a >= b) return true;

    return false;
}

bool logic_worker::ready_for_game(std::string room_key)
{
    thread_sync sync;

    auto iter = room_hashs_.find(room_key);

    if (iter != room_hashs_.end())
        iter->second.ready_player_num_++;
    else
        return false;

    return true;
}

logic_worker::HOLDEM_HANDS logic_worker::check_card_mix(int i, int j, int k)
{
    thread_sync sync;

    if (i == j)
        if (i == k)
            return HOLDEM_HANDS::TRIPLE;

    if (i == j || i == k || j == k)
        return HOLDEM_HANDS::PAIR;

    int iarray[3] = {i, j, k};
    std::sort(iarray, iarray + 2, comp);

    if (iarray[0] + 1 == iarray[1])
        if (iarray[1] + 1 == iarray[2])
            return HOLDEM_HANDS::STRAIGHT;

    return HOLDEM_HANDS::NONE;
}

void logic_worker::process_queue()
{
    while (end_server_)
    {
        thread_sync sync;

        if (room_hashs_.empty())
            continue;
        
        auto iter = room_hashs_.begin();

        while (iter != room_hashs_.end())
        {
            switch (iter->second.state_)
            {
            case ROOM_INFO::READY:
            {
                if (iter->second.ready_player_num_ == 2)
                {
                    iter->second.state_ = ROOM_INFO::PLAYING;

                    logic_server::packet_game_state_ntf start_ntf_packet;

                    start_ntf_packet.set_win_player_key("temp");
                    start_ntf_packet.set_state(1);

                    iter->second.player_[0].session_->handle_send(logic_server::GAME_STATE_NTF, start_ntf_packet);
                    iter->second.player_[1].session_->handle_send(logic_server::GAME_STATE_NTF, start_ntf_packet);

                    log(iter->first)->info("game start, room_key:{}, player_1_key:{}, player_2_key:{}",
                        iter->first,
                        iter->second.player_[0].session_->get_player_key(),
                        iter->second.player_[1].session_->get_player_key()
                    );

                    logic_server::packet_process_turn_ntf turn_ntf_packet;
                    
                    iter->second.public_card_[0] = iter->second.get_card();
                    iter->second.public_card_[1] = iter->second.get_card();
                    turn_ntf_packet.set_public_card_number_1(iter->second.public_card_[0]);
                    turn_ntf_packet.set_public_card_number_2(iter->second.public_card_[1]);

                    for (int i = 0; i < 2; i++)
                    {
                        iter->second.player_[i].opponent_card_num_ = iter->second.get_card();
                                                
                        turn_ntf_packet.set_opponent_card_number(iter->second.player_[i].opponent_card_num_);
                        turn_ntf_packet.set_remain_money(iter->second.player_[i].remain_money_);
                        turn_ntf_packet.set_opponent_money(0);
                        turn_ntf_packet.set_my_money(0);

                        iter->second.player_[i].session_->handle_send(logic_server::PROCESS_TURN_NTF, turn_ntf_packet);
                    }

                    logic_server::packet_process_turn_req turn_req_packet;

                    if (random_generator::get_random_int(0, 100) < 50)
                    {
                        iter->second.turn_player_ = &iter->second.player_[0];

                        turn_req_packet.set_my_money(iter->second.player_[0].sum_money_);
                        turn_req_packet.set_opponent_money(iter->second.player_[1].sum_money_);
                    }
                    else
                    {
                        iter->second.turn_player_ = &iter->second.player_[1];

                        turn_req_packet.set_my_money(iter->second.player_[1].sum_money_);
                        turn_req_packet.set_opponent_money(iter->second.player_[0].sum_money_);
                    }

                    iter->second.turn_player_->session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);
                }
            }
            break;

            case ROOM_INFO::PLAYING:
            {
                if (iter->second.turn_player_->submit_card_)
                {
                    iter->second.turn_player_->submit_card_ = false;

                    iter->second.turn_player_->sum_money_ += iter->second.turn_player_->submit_money_;
              
                    enum TURN_TYPE { TURN_PASS = 0, CHECK_CARD, GIVE_UP, END_TURN };

                    TURN_TYPE turn_type = TURN_PASS;
                                        
                    if (iter->second.player_[0].sum_money_ == iter->second.player_[1].sum_money_)
                    {
                        if (iter->second.hide_card_ == true)
                            turn_type = CHECK_CARD;
                        else
                            turn_type = END_TURN;
                    }
                    
                    if (iter->second.turn_player_->submit_money_ == 0)
                        turn_type = GIVE_UP;

                    if (iter->second.turn_player_->remain_money_ - iter->second.turn_player_->sum_money_ <= 0)
                        turn_type = END_TURN;
                    
                    if (turn_type == TURN_PASS)
                    {
                        logic_server::packet_process_turn_req turn_req_packet;

                        if (iter->second.turn_player_ == &iter->second.player_[0])
                        {
                            log(iter->first)->info("turn_type:{}, submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                turn_type,
                                iter->second.turn_player_->session_->get_player_key(),
                                iter->second.turn_player_->sum_money_,
                                iter->second.player_[1].sum_money_
                            );

                            iter->second.turn_player_ = &iter->second.player_[1];
                        
                            turn_req_packet.set_my_money(iter->second.player_[1].sum_money_);
                            turn_req_packet.set_opponent_money(iter->second.player_[0].sum_money_);
                        }
                        else
                        {
                            log(iter->first)->info("turn_type:{}, submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                turn_type,
                                iter->second.turn_player_->session_->get_player_key(),
                                iter->second.turn_player_->sum_money_,
                                iter->second.player_[0].sum_money_
                            );

                            iter->second.turn_player_ = &iter->second.player_[0];
                        
                            turn_req_packet.set_my_money(iter->second.player_[0].sum_money_);
                            turn_req_packet.set_opponent_money(iter->second.player_[1].sum_money_);
                        }
                        
                        iter->second.turn_player_->session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);
                    }
                    else if (turn_type == CHECK_CARD)
                    {
                        iter->second.hide_card_ = false;
                        
                        logic_server::packet_process_turn_req turn_req_packet;

                        if (iter->second.turn_player_ == &iter->second.player_[0])
                        {
                            log(iter->first)->info("turn_type:{}, submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                turn_type,
                                iter->second.turn_player_->session_->get_player_key(),
                                iter->second.turn_player_->sum_money_,
                                iter->second.player_[1].sum_money_
                            );

                            iter->second.turn_player_ = &iter->second.player_[1];

                            turn_req_packet.set_my_money(iter->second.player_[1].sum_money_);
                            turn_req_packet.set_opponent_money(iter->second.player_[0].sum_money_);
                        }
                        else
                        {
                            log(iter->first)->info("turn_type:{}, submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                turn_type,
                                iter->second.turn_player_->session_->get_player_key(),
                                iter->second.turn_player_->sum_money_,
                                iter->second.player_[0].sum_money_
                            );

                            iter->second.turn_player_ = &iter->second.player_[0];
                        
                            turn_req_packet.set_my_money(iter->second.player_[0].sum_money_);
                            turn_req_packet.set_opponent_money(iter->second.player_[1].sum_money_);
                        }
                        
                        iter->second.turn_player_->session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);

                        logic_server::packet_process_check_card_ntf check_card_packet;
                        check_card_packet.set_result(1);

                        for (int i = 0; i < 2; i++)
                            iter->second.player_[i].session_->handle_send(logic_server::PROCESS_CHECK_CARD_NTF, check_card_packet);
                    }
                    else if (turn_type == GIVE_UP)
                    {                        
                        if (iter->second.turn_player_ == &iter->second.player_[0])
                        {
                            iter->second.player_[0].remain_money_ -= iter->second.player_[0].sum_money_;
                            iter->second.player_[1].remain_money_ += iter->second.player_[0].sum_money_;

                            log(iter->first)->info("turn_type:{}, submit_player:{}, remain_coin:{}, other_player_remain_coin:{}",
                                turn_type,
                                iter->second.turn_player_->session_->get_player_key(),
                                iter->second.turn_player_->remain_money_,
                                iter->second.player_[1].remain_money_
                            );
                        }
                        else
                        {
                            iter->second.player_[0].remain_money_ += iter->second.player_[1].sum_money_;
                            iter->second.player_[1].remain_money_ -= iter->second.player_[1].sum_money_;

                            log(iter->first)->info("turn_type:{}, submit_player:{}, remain_coin:{}, other_player_remain_coin:{}",
                                turn_type,
                                iter->second.turn_player_->session_->get_player_key(),
                                iter->second.turn_player_->remain_money_,
                                iter->second.player_[0].remain_money_
                            );
                        }

                        for (int i = 0; i < 2; i++)
                            iter->second.player_[i].sum_money_ = 1;

                        iter->second.hide_card_ = true;
                        
                        logic_server::packet_process_turn_ntf turn_ntf_packet;

                        iter->second.public_card_[0] = iter->second.get_card();
                        iter->second.public_card_[1] = iter->second.get_card();
                        turn_ntf_packet.set_public_card_number_1(iter->second.public_card_[0]);
                        turn_ntf_packet.set_public_card_number_2(iter->second.public_card_[1]);

                        for (int i = 0; i < 2; i++)
                        {
                            iter->second.player_[i].opponent_card_num_ = iter->second.get_card();

                            turn_ntf_packet.set_opponent_card_number(iter->second.player_[i].opponent_card_num_);
                            turn_ntf_packet.set_remain_money(iter->second.player_[i].remain_money_);
                            turn_ntf_packet.set_opponent_money(0);
                            turn_ntf_packet.set_my_money(0);

                            iter->second.player_[i].session_->handle_send(logic_server::PROCESS_TURN_NTF, turn_ntf_packet);
                        }

                        logic_server::packet_process_turn_req turn_req_packet;

                        if (iter->second.turn_player_ == &iter->second.player_[0])
                        {
                            iter->second.turn_player_ = &iter->second.player_[1];

                            turn_req_packet.set_my_money(iter->second.player_[1].sum_money_);
                            turn_req_packet.set_opponent_money(iter->second.player_[0].sum_money_);
                        }
                        else
                        {
                            iter->second.turn_player_ = &iter->second.player_[0];

                            turn_req_packet.set_my_money(iter->second.player_[0].sum_money_);
                            turn_req_packet.set_opponent_money(iter->second.player_[1].sum_money_);
                        }

                        iter->second.turn_player_->session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);
                    }
                    else if (turn_type == END_TURN) {
                        
                        HOLDEM_HANDS player_1_result = check_card_mix(
                            iter->second.player_[1].opponent_card_num_,
                            iter->second.public_card_[0],
                            iter->second.public_card_[1]
                        );

                        HOLDEM_HANDS player_2_result = check_card_mix(
                            iter->second.player_[0].opponent_card_num_,
                            iter->second.public_card_[0],
                            iter->second.public_card_[1]
                        );

                        log(iter->first)->info("turn_type:{}",
                            turn_type
                        );

                        log(iter->first)->info("-> player:{}, card_deck:{}, card_num:{}",
                            iter->second.player_[0].session_->get_player_key(),
                            player_1_result,
                            iter->second.player_[1].opponent_card_num_
                        );

                        log(iter->first)->info("-> player:{}, card_deck:{}, card_num:{}",
                            iter->second.player_[1].session_->get_player_key(),
                            player_2_result,
                            iter->second.player_[0].opponent_card_num_
                        );

                        bool completely_same = true;

                        if (player_1_result > player_2_result)
                        {
                            iter->second.player_[0].remain_money_ += iter->second.player_[1].sum_money_;
                            iter->second.player_[1].remain_money_ -= iter->second.player_[1].sum_money_;
                        
                            completely_same = false;
                        }
                        else if (player_1_result < player_2_result)
                        {
                            iter->second.player_[0].remain_money_ -= iter->second.player_[0].sum_money_;
                            iter->second.player_[1].remain_money_ += iter->second.player_[0].sum_money_;
                        
                            completely_same = false;
                        }
                        else 
                        {    
                            if (iter->second.player_[0].opponent_card_num_ > iter->second.player_[1].opponent_card_num_)
                            {
                                iter->second.player_[1].remain_money_ += iter->second.player_[0].sum_money_;
                                iter->second.player_[0].remain_money_ -= iter->second.player_[0].sum_money_;
                                
                                completely_same = false;
                            }
                            else if (iter->second.player_[0].opponent_card_num_ < iter->second.player_[1].opponent_card_num_)
                            {
                                iter->second.player_[1].remain_money_ -= iter->second.player_[1].sum_money_;
                                iter->second.player_[0].remain_money_ += iter->second.player_[1].sum_money_;
                                
                                completely_same = false;
                            }
                        }

                        if (iter->second.player_[0].remain_money_ <= 0 || iter->second.player_[1].remain_money_ <= 0)
                        {
                            iter->second.state_ = ROOM_INFO::END;
                            break;
                        }
                        
                        if (completely_same)
                        {
                            logic_server::packet_process_turn_ntf turn_ntf_packet;

                            iter->second.public_card_[0] = iter->second.get_card();
                            iter->second.public_card_[1] = iter->second.get_card();

                            turn_ntf_packet.set_public_card_number_1(iter->second.public_card_[0]);
                            turn_ntf_packet.set_public_card_number_2(iter->second.public_card_[1]);

                            for (int i = 0; i < 2; i++)
                            {
                                iter->second.player_[i].opponent_card_num_ = iter->second.get_card();

                                turn_ntf_packet.set_opponent_card_number(iter->second.player_[i].opponent_card_num_);
                                turn_ntf_packet.set_remain_money(iter->second.player_[i].remain_money_);

                                if (i == 0)
                                {
                                    turn_ntf_packet.set_opponent_money(iter->second.player_[1].sum_money_++);
                                    turn_ntf_packet.set_my_money(iter->second.player_[0].sum_money_);
                                }
                                else
                                {
                                    turn_ntf_packet.set_opponent_money(iter->second.player_[0].sum_money_++);
                                    turn_ntf_packet.set_my_money(iter->second.player_[1].sum_money_);
                                }

                                iter->second.player_[i].session_->handle_send(logic_server::PROCESS_TURN_NTF, turn_ntf_packet);
                            }

                            log(iter->first)->info("-> completely_same:true, player_1:{}, player_2:{}",
                                iter->second.player_[0].remain_money_,
                                iter->second.player_[1].remain_money_
                            );
                        }
                        else
                        {
                            if (iter->second.player_[0].remain_money_ <= 0 || iter->second.player_[1].remain_money_ <= 0)
                            {
                                iter->second.state_ = ROOM_INFO::END;
                                break;
                            }

                            logic_server::packet_process_turn_ntf turn_ntf_packet;

                            iter->second.public_card_[0] = iter->second.get_card();
                            iter->second.public_card_[1] = iter->second.get_card();
                            turn_ntf_packet.set_public_card_number_1(iter->second.public_card_[0]);
                            turn_ntf_packet.set_public_card_number_2(iter->second.public_card_[1]);

                            for (int i = 0; i < 2; i++)
                            {
                                iter->second.player_[i].opponent_card_num_ = iter->second.get_card();

                                turn_ntf_packet.set_opponent_card_number(iter->second.player_[i].opponent_card_num_);
                                turn_ntf_packet.set_remain_money(iter->second.player_[i].remain_money_);
                                turn_ntf_packet.set_opponent_money(0);
                                turn_ntf_packet.set_my_money(0);

                                iter->second.player_[i].session_->handle_send(logic_server::PROCESS_TURN_NTF, turn_ntf_packet);
                            }

                            for (int i = 0; i < 2; i++)
                                iter->second.player_[i].sum_money_ = 1;

                            log(iter->first)->info("-> completely_same:false",
                                turn_type
                            );
                        }

                        iter->second.hide_card_ = true;

                        logic_server::packet_process_turn_req turn_req_packet;

                        if (iter->second.turn_player_ == &iter->second.player_[0])
                        {
                            log(iter->first)->info("-> submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                iter->second.turn_player_->session_->get_player_key(),
                                iter->second.turn_player_->sum_money_,
                                iter->second.player_[1].sum_money_
                            );

                            iter->second.turn_player_ = &iter->second.player_[1];

                            turn_req_packet.set_my_money(iter->second.player_[1].sum_money_);
                            turn_req_packet.set_opponent_money(iter->second.player_[0].sum_money_);
                        }
                        else
                        {
                            log(iter->first)->info("-> submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                iter->second.turn_player_->session_->get_player_key(),
                                iter->second.turn_player_->sum_money_,
                                iter->second.player_[0].sum_money_
                            );

                            iter->second.turn_player_ = &iter->second.player_[0];

                            turn_req_packet.set_my_money(iter->second.player_[0].sum_money_);
                            turn_req_packet.set_opponent_money(iter->second.player_[1].sum_money_);
                        }

                        iter->second.turn_player_->session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);
                    }

                }
            }
            break;

            case ROOM_INFO::END:
            {
                logic_server::packet_game_state_ntf game_state_packet;

                game_state_packet.set_state(2);

                int disconnect_session = 0;

                for (int i = 0; i < 2; i++)
                {
                    if (!iter->second.player_[i].session_->is_connected())
                    {
                        disconnect_session++;
                        continue;
                    }

                    iter->second.player_[i].session_->handle_send(logic_server::GAME_STATE_NTF, game_state_packet);
                
                    iter->second.player_[i].session_->shut_down();
                }

                if (disconnect_session >= 2)
                {
                    if (iter->second.player_[0].remain_money_ != 0)
                    {
                        log(iter->first)->info("end_game, win_player_key:{}",
                            iter->second.player_[0].session_->get_player_key()
                        );
                    }
                    else
                    {
                        log(iter->first)->info("end_game, win_player_key:{}",
                            iter->second.player_[1].session_->get_player_key()
                        );
                    }

                    if (iter->second.player_[0].remain_money_ >= iter->second.player_[1].remain_money_)
                    {
                        db_query query;
                        query.callback_func = nullptr;
                        query.query_ = "UPDATE user_info SET ";
                        query.query_ += "win = win + 1, ";
                        query.query_ += "rating = rating + 20 ";
                        query.query_ += "WHERE id = \'";
                        query.query_ += redis_connector::get_instance()->get_id(iter->second.player_[0].session_->get_player_key()) + "\';";

                        db_connector->push_query(query);

                        db_query query2;
                        query2.callback_func = nullptr;
                        query2.query_ = "UPDATE user_info SET ";
                        query2.query_ += "lose = lose + 1, ";
                        query2.query_ += "rating = rating - 20 ";
                        query2.query_ += "WHERE id = \'";
                        query2.query_ += redis_connector::get_instance()->get_id(iter->second.player_[1].session_->get_player_key()) + "\';";

                        db_connector->push_query(query2);

                        game_state_packet.set_win_player_key(iter->second.player_[0].session_->get_player_key());

                    }
                    else
                    {
                        db_query query;
                        query.callback_func = nullptr;
                        query.query_ = "UPDATE user_info SET ";
                        query.query_ += "win = win + 1, ";
                        query.query_ += "rating = rating + 20 ";
                        query.query_ += "WHERE id = \'";
                        query.query_ += redis_connector::get_instance()->get_id(iter->second.player_[1].session_->get_player_key()) + "\';";

                        db_connector->push_query(query);

                        db_query query2;
                        query2.callback_func = nullptr;
                        query2.query_ = "UPDATE user_info SET ";
                        query2.query_ += "lose = lose + 1, ";
                        query2.query_ += "rating = rating - 20 ";
                        query2.query_ += "WHERE id = \'";
                        query2.query_ += redis_connector::get_instance()->get_id(iter->second.player_[0].session_->get_player_key()) + "\';";

                        db_connector->push_query(query2);

                        game_state_packet.set_win_player_key(iter->second.player_[1].session_->get_player_key());
                    }

                    iter = room_hashs_.erase(iter);
                }
            }
            break;
            }

            if (iter != room_hashs_.end())
                iter++;
        }
    }
}

bool logic_worker::disconnect_room(std::string room_key, std::string player_key)
{
    thread_sync sync;

    auto iter = room_hashs_.find(room_key);

    if (iter == room_hashs_.end())
        return false;

    system_log->info("remove room info - room_key:{}", room_key);

    std::string win_player_key;

    if (iter->second.player_[0].session_->get_player_key() == player_key)
        win_player_key = iter->second.player_[1].session_->get_player_key();
    else
        win_player_key = iter->second.player_[0].session_->get_player_key();

    logic_server::packet_game_state_ntf game_state_packet;

    game_state_packet.set_state(2);
    game_state_packet.set_win_player_key(win_player_key);

    db_query query;
    query.callback_func = nullptr;
    query.query_ = "UPDATE user_info SET ";
    query.query_ += "win = win + 1, ";
    query.query_ += "rating = rating + 20 ";
    query.query_ += "WHERE id = \'";
    query.query_ += redis_connector::get_instance()->get_id(win_player_key) + "\';";

    db_connector->push_query(query);

    db_query query2;
    query2.callback_func = nullptr;
    query2.query_ = "UPDATE user_info SET ";
    query2.query_ += "lose = lose + 1, ";
    query2.query_ += "rating = rating - 20 ";
    query2.query_ += "WHERE id = \'";
    query2.query_ += redis_connector::get_instance()->get_id(player_key) + "\';";

    db_connector->push_query(query2);
        
    int disconnect_session = 0;

    for (int i = 0; i < 2; i++)
    {
        iter->second.player_[i].session_->handle_send(logic_server::GAME_STATE_NTF, game_state_packet);
        iter->second.player_[i].session_->shut_down();
    
        if (!iter->second.player_[i].session_->is_connected())
            disconnect_session++;
    }

    if (disconnect_session >= 2)
    {
        log(iter->first)->info("end_game, win_player_key:{}", win_player_key);
      
        iter = room_hashs_.erase(iter);

        return true;
    }

    return false;
}

bool logic_worker::destroy_room(std::string room_key)
{
    thread_sync sync;

    if (redis_connector::get_instance()->remove_room_info(room_key))
        return true;

    return false;
}
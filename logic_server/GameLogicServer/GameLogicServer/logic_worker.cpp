#include "logic_worker.h"
#include "random_generator.h"
#include "log_manager.h"

_PLAYER_INFO::_PLAYER_INFO()
{
    submit_money_= 0;
    sum_money_ = 0;
    remain_money_ = 20;
    submit_card_ = false;
}

_ROOM_INFO::_ROOM_INFO()
{
    ready_player_num_ = 1;
    time_ = 0;
    turn_count_ = 0;
    hide_card_ = true;
    state_ = GAME_STATE::READY;
}

void _ROOM_INFO::generate_card_queue()
{
    while (!card_list_.empty())
        card_list_.pop();

    std::deque<int> d;
    
    for (int i = 0; i < 4; i++)
        for (int j = 1; j <= 10; j++)
            d.push_back(j);

    srand((unsigned int)time(NULL));
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

logic_worker::logic_worker() : logic_thread_(NULL)
{}

logic_worker::~logic_worker() 
{}

bool logic_worker::init_singleton()
{
    thread_sync sync;

    if (logic_thread_ == NULL)
        logic_thread_ = new boost::thread(&logic_worker::process_queue, this);

    end_server_ = true;

    return true;
}

bool logic_worker::release_singleton()
{
    end_server_ = false;

    logic_thread_->join();

    return true;
}

bool logic_worker::create_room(connected_session* session, std::string room_key, int player_key)
{
    thread_sync sync;

    if (!redis_connector::get_instance()->check_room(room_key))
        return false;
    
    ROOM_INFO room_info;

    room_info.player_[0].key_ = player_key;
    room_info.player_[0].session_ = session;
    room_info.player_[0].remain_money_ = 20;
    room_info.player_[0].sum_money_ = 1;
    room_info.player_[0].submit_money_ = 0;

    room_info.room_key_ = room_key;
    
    room_list_.push_back(room_info);

    return true;
}

bool logic_worker::enter_room_player(connected_session* session, std::string room_key, int player_key)
{
    thread_sync sync;
    
    for (auto iter = room_list_.begin(); iter != room_list_.end(); iter++)
    {
        if ((*iter).room_key_ == room_key)
        {
            (*iter).ready_player_num_++;
            (*iter).player_[1].remain_money_ = 20;
            (*iter).player_[1].submit_money_ = 0;
            (*iter).player_[1].sum_money_ = 1;
            (*iter).player_[1].key_ = player_key;
            (*iter).player_[1].session_ = session;

            return true;
        }
    }

    if (!create_room(session, room_key, player_key))
        return false;

    return true;
}

bool logic_worker::process_turn(int player_key, int money)
{
    thread_sync sync;

    for (auto iter = room_list_.begin(); iter != room_list_.end(); iter++)
    {
        if ((*iter).player_[0].key_ == player_key)
        {
            (*iter).player_[0].submit_card_ = true;
            (*iter).player_[0].submit_money_ = money;

            return true;
        }
        else if ((*iter).player_[1].key_ == player_key)
        {
            (*iter).player_[1].submit_card_ = true;
            (*iter).player_[1].submit_money_ = money;

            return true;
        }
    }

    return false;
}

bool comp(int const& a, int const& b) {
    if (a >= b) return true;

    return false;
}

logic_worker::HOLDEM_HANDS logic_worker::check_card_mix(int i, int j, int k)
{
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

        for (auto iter = room_list_.begin(); iter != room_list_.end(); iter++)
        {
            switch ((*iter).state_)
            {
            case ROOM_INFO::READY:
            {
                if ((*iter).ready_player_num_ == 2)
                {
                    (*iter).state_ = ROOM_INFO::PLAYING;

                    logic_server::packet_game_state_ntf start_ntf_packet;

                    start_ntf_packet.set_win_player_key(0);
                    start_ntf_packet.set_state(1);

                    (*iter).player_[0].session_->handle_send(logic_server::GAME_STATE_NTF, start_ntf_packet);
                    (*iter).player_[1].session_->handle_send(logic_server::GAME_STATE_NTF, start_ntf_packet);

                    log((*iter).room_key_)->info("game start, room_key:{}, player_1_key:{}, player_2_key:{}",
                        (*iter).room_key_,
                        (*iter).player_[0].key_,
                        (*iter).player_[1].key_
                    );

                    logic_server::packet_process_turn_ntf turn_ntf_packet;
                    
                    (*iter).public_card_[0] = (*iter).get_card();
                    (*iter).public_card_[1] = (*iter).get_card();
                    turn_ntf_packet.set_public_card_number_1((*iter).public_card_[0]);
                    turn_ntf_packet.set_public_card_number_2((*iter).public_card_[1]);

                    for (int i = 0; i < 2; i++)
                    {
                        (*iter).player_[i].opponent_card_num_ = (*iter).get_card();
                                                
                        turn_ntf_packet.set_opponent_card_number((*iter).player_[i].opponent_card_num_);
                        turn_ntf_packet.set_remain_money((*iter).player_[i].remain_money_);
                        turn_ntf_packet.set_opponent_money(0);
                        turn_ntf_packet.set_my_money(0);

                        (*iter).player_[i].session_->handle_send(logic_server::PROCESS_TURN_NTF, turn_ntf_packet);
                    }

                    logic_server::packet_process_turn_req turn_req_packet;

                    if (random_generator::get_random_int(0, 100) < 50)
                    {
                        (*iter).turn_player_ = &(*iter).player_[0];

                        turn_req_packet.set_my_money((*iter).player_[0].sum_money_);
                        turn_req_packet.set_opponent_money((*iter).player_[1].sum_money_);
                    }
                    else
                    {
                        (*iter).turn_player_ = &(*iter).player_[1];

                        turn_req_packet.set_my_money((*iter).player_[1].sum_money_);
                        turn_req_packet.set_opponent_money((*iter).player_[0].sum_money_);
                    }

                    (*iter).turn_player_->session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);
                }
            }
            break;

            case ROOM_INFO::PLAYING:
            {
                if ((*iter).turn_player_->submit_card_)
                {
                    (*iter).turn_player_->submit_card_ = false;

                    (*iter).turn_player_->sum_money_ += (*iter).turn_player_->submit_money_;
                    (*iter).turn_player_->remain_money_ -= (*iter).turn_player_->submit_money_;

                    enum TURN_TYPE { TURN_PASS = 0, CHECK_CARD, GIVE_UP, END_TURN };

                    TURN_TYPE turn_type = TURN_PASS;
                    
                    if ((*iter).turn_player_->submit_money_ == 0)
                        turn_type = GIVE_UP;
                    
                    if ((*iter).player_[0].sum_money_ == (*iter).player_[1].sum_money_)
                        if ((*iter).hide_card_ == true)
                            turn_type = CHECK_CARD;
                        else
                            turn_type = END_TURN;
                    
                    if (turn_type == TURN_PASS)
                    {
                        logic_server::packet_process_turn_req turn_req_packet;

                        if ((*iter).turn_player_ == &(*iter).player_[0])
                        {
                            log((*iter).room_key_)->info("turn_type:{}, submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                turn_type,
                                (*iter).turn_player_->key_,
                                (*iter).turn_player_->sum_money_,
                                (*iter).player_[1].sum_money_
                            );

                            (*iter).turn_player_ = &(*iter).player_[1];
                        
                            turn_req_packet.set_my_money((*iter).player_[1].sum_money_);
                            turn_req_packet.set_opponent_money((*iter).player_[0].sum_money_);
                        }
                        else
                        {
                            log((*iter).room_key_)->info("turn_type:{}, submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                turn_type,
                                (*iter).turn_player_->key_,
                                (*iter).turn_player_->sum_money_,
                                (*iter).player_[0].sum_money_
                            );

                            (*iter).turn_player_ = &(*iter).player_[0];
                        
                            turn_req_packet.set_my_money((*iter).player_[0].sum_money_);
                            turn_req_packet.set_opponent_money((*iter).player_[1].sum_money_);
                        }
                        
                        (*iter).turn_player_->session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);
                    }
                    else if (turn_type == CHECK_CARD)
                    {
                        (*iter).hide_card_ = false;
                        
                        logic_server::packet_process_turn_req turn_req_packet;

                        if ((*iter).turn_player_ == &(*iter).player_[0])
                        {
                            log((*iter).room_key_)->info("turn_type:{}, submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                turn_type,
                                (*iter).turn_player_->key_,
                                (*iter).turn_player_->sum_money_,
                                (*iter).player_[1].sum_money_
                            );

                            (*iter).turn_player_ = &(*iter).player_[1];

                            turn_req_packet.set_my_money((*iter).player_[1].sum_money_);
                            turn_req_packet.set_opponent_money((*iter).player_[0].sum_money_);
                        }
                        else
                        {
                            log((*iter).room_key_)->info("turn_type:{}, submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                turn_type,
                                (*iter).turn_player_->key_,
                                (*iter).turn_player_->sum_money_,
                                (*iter).player_[0].sum_money_
                            );

                            (*iter).turn_player_ = &(*iter).player_[0];
                        
                            turn_req_packet.set_my_money((*iter).player_[0].sum_money_);
                            turn_req_packet.set_opponent_money((*iter).player_[1].sum_money_);
                        }
                        
                        (*iter).turn_player_->session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);

                        logic_server::packet_process_check_card_ntf check_card_packet;
                        check_card_packet.set_result(1);

                        for (int i = 0; i < 2; i++)
                            (*iter).player_[i].session_->handle_send(logic_server::PROCESS_CHECK_CARD_NTF, check_card_packet);
                    }
                    else if (turn_type == GIVE_UP)
                    {                        
                        if ((*iter).turn_player_ == &(*iter).player_[0])
                        {
                            (*iter).player_[0].remain_money_ -= (*iter).player_[0].sum_money_;
                            (*iter).player_[1].remain_money_ += (*iter).player_[0].sum_money_ + (*iter).player_[1].sum_money_;

                            log((*iter).room_key_)->info("turn_type:{}, submit_player:{}, remain_coin:{}, other_player_remain_coin:{}",
                                turn_type,
                                (*iter).turn_player_->key_,
                                (*iter).turn_player_->remain_money_,
                                (*iter).player_[1].remain_money_
                            );
                        }
                        else
                        {
                            (*iter).player_[0].remain_money_ += (*iter).player_[0].sum_money_ + (*iter).player_[1].sum_money_;
                            (*iter).player_[1].remain_money_ -= (*iter).player_[1].sum_money_;

                            log((*iter).room_key_)->info("turn_type:{}, submit_player:{}, remain_coin:{}, other_player_remain_coin:{}",
                                turn_type,
                                (*iter).turn_player_->key_,
                                (*iter).turn_player_->remain_money_,
                                (*iter).player_[0].remain_money_
                            );
                        }

                        for (int i = 0; i < 2; i++)
                            (*iter).player_[i].sum_money_ = 1;

                        (*iter).hide_card_ = true;

                        
                        if ((*iter).player_[0].remain_money_ <= 0 || (*iter).player_[1].remain_money_ <= 0)
                        {
                            (*iter).state_ = ROOM_INFO::END;
                            break;
                        }

                        logic_server::packet_process_turn_ntf turn_ntf_packet;

                        (*iter).public_card_[0] = (*iter).get_card();
                        (*iter).public_card_[1] = (*iter).get_card();
                        turn_ntf_packet.set_public_card_number_1((*iter).public_card_[0]);
                        turn_ntf_packet.set_public_card_number_2((*iter).public_card_[1]);

                        for (int i = 0; i < 2; i++)
                        {
                            (*iter).player_[i].opponent_card_num_ = (*iter).get_card();

                            turn_ntf_packet.set_opponent_card_number((*iter).player_[i].opponent_card_num_);
                            turn_ntf_packet.set_remain_money((*iter).player_[i].remain_money_);
                            turn_ntf_packet.set_opponent_money(0);
                            turn_ntf_packet.set_my_money(0);

                            (*iter).player_[i].session_->handle_send(logic_server::PROCESS_TURN_NTF, turn_ntf_packet);
                        }

                        logic_server::packet_process_turn_req turn_req_packet;

                        if ((*iter).turn_player_ == &(*iter).player_[0])
                        {
                            (*iter).turn_player_ = &(*iter).player_[1];

                            turn_req_packet.set_my_money((*iter).player_[1].sum_money_);
                            turn_req_packet.set_opponent_money((*iter).player_[0].sum_money_);
                        }
                        else
                        {
                            (*iter).turn_player_ = &(*iter).player_[0];

                            turn_req_packet.set_my_money((*iter).player_[0].sum_money_);
                            turn_req_packet.set_opponent_money((*iter).player_[1].sum_money_);
                        }

                        (*iter).turn_player_->session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);
                    }
                    else if (turn_type == END_TURN) {
                        
                        HOLDEM_HANDS player_1_result = check_card_mix(
                            (*iter).player_[1].opponent_card_num_,
                            (*iter).public_card_[0],
                            (*iter).public_card_[1]
                        );

                        HOLDEM_HANDS player_2_result = check_card_mix(
                            (*iter).player_[0].opponent_card_num_,
                            (*iter).public_card_[0],
                            (*iter).public_card_[1]
                        );

                        log((*iter).room_key_)->info("turn_type:{}",
                            turn_type
                        );

                        log((*iter).room_key_)->info("-> player:{}, card_deck:{}, card_num:{}",
                            (*iter).player_[0].key_,
                            player_1_result,
                            (*iter).player_[1].opponent_card_num_
                        );

                        log((*iter).room_key_)->info("-> player:{}, card_deck:{}, card_num:{}",
                            (*iter).player_[1].key_,
                            player_2_result,
                            (*iter).player_[0].opponent_card_num_
                        );

                        bool completely_same = true;

                        if (player_1_result > player_2_result)
                        {
                            (*iter).player_[0].remain_money_ += (*iter).player_[0].sum_money_ + (*iter).player_[1].sum_money_;
                            (*iter).player_[1].remain_money_ -= (*iter).player_[1].sum_money_;
                        
                            completely_same = false;
                        }
                        else if (player_1_result < player_2_result)
                        {
                            (*iter).player_[0].remain_money_ -= (*iter).player_[0].sum_money_;
                            (*iter).player_[1].remain_money_ += (*iter).player_[0].sum_money_ + (*iter).player_[1].sum_money_;
                        
                            completely_same = false;
                        }
                        else 
                        {    
                            if ((*iter).player_[0].opponent_card_num_ > (*iter).player_[1].opponent_card_num_)
                            {
                                (*iter).player_[1].remain_money_ += (*iter).player_[0].sum_money_ + (*iter).player_[1].sum_money_;
                                (*iter).player_[0].remain_money_ -= (*iter).player_[0].sum_money_;
                                
                                completely_same = false;
                            }
                            else if ((*iter).player_[0].opponent_card_num_ < (*iter).player_[1].opponent_card_num_)
                            {
                                (*iter).player_[1].remain_money_ -= (*iter).player_[1].sum_money_;
                                (*iter).player_[0].remain_money_ += (*iter).player_[0].sum_money_ + (*iter).player_[1].sum_money_;
                                
                                completely_same = false;
                            }
                        
                            if (completely_same)
                            {
                                logic_server::packet_process_turn_ntf turn_ntf_packet;

                                (*iter).public_card_[0] = (*iter).get_card();
                                (*iter).public_card_[1] = (*iter).get_card();

                                turn_ntf_packet.set_public_card_number_1((*iter).public_card_[0]);
                                turn_ntf_packet.set_public_card_number_2((*iter).public_card_[1]);

                                for (int i = 0; i < 2; i++)
                                {
                                    (*iter).player_[i].opponent_card_num_ = (*iter).get_card();

                                    turn_ntf_packet.set_opponent_card_number((*iter).player_[i].opponent_card_num_);
                                    turn_ntf_packet.set_remain_money((*iter).player_[i].remain_money_);
                                    
                                    if (i == 0)
                                    {
                                        turn_ntf_packet.set_opponent_money((*iter).player_[1].sum_money_);
                                        turn_ntf_packet.set_my_money((*iter).player_[0].sum_money_);
                                    }
                                    else
                                    {
                                        turn_ntf_packet.set_opponent_money((*iter).player_[0].sum_money_);
                                        turn_ntf_packet.set_my_money((*iter).player_[1].sum_money_);
                                    }

                                    (*iter).player_[i].session_->handle_send(logic_server::PROCESS_TURN_NTF, turn_ntf_packet);
                                }

                                log((*iter).room_key_)->info("-> completely_same:true",
                                    turn_type
                                );
                            }
                        }

                        if (!completely_same)
                        {
                            if ((*iter).player_[0].remain_money_ <= 0 || (*iter).player_[1].remain_money_ <= 0)
                            {
                                (*iter).state_ = ROOM_INFO::END;
                                break;
                            }

                            logic_server::packet_process_turn_ntf turn_ntf_packet;

                            (*iter).public_card_[0] = (*iter).get_card();
                            (*iter).public_card_[1] = (*iter).get_card();
                            turn_ntf_packet.set_public_card_number_1((*iter).public_card_[0]);
                            turn_ntf_packet.set_public_card_number_2((*iter).public_card_[1]);

                            for (int i = 0; i < 2; i++)
                            {
                                (*iter).player_[i].opponent_card_num_ = (*iter).get_card();

                                turn_ntf_packet.set_opponent_card_number((*iter).player_[i].opponent_card_num_);
                                turn_ntf_packet.set_remain_money((*iter).player_[i].remain_money_);
                                turn_ntf_packet.set_opponent_money(0);
                                turn_ntf_packet.set_my_money(0);

                                (*iter).player_[i].session_->handle_send(logic_server::PROCESS_TURN_NTF, turn_ntf_packet);
                            }

                            for (int i = 0; i < 2; i++)
                                (*iter).player_[i].sum_money_ = 1;

                            log((*iter).room_key_)->info("-> completely_same:false",
                                turn_type
                            );
                        }

                        (*iter).hide_card_ = true;

                        logic_server::packet_process_turn_req turn_req_packet;

                        if ((*iter).turn_player_ == &(*iter).player_[0])
                        {
                            log((*iter).room_key_)->info("-> submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                turn_type,
                                (*iter).turn_player_->key_,
                                (*iter).turn_player_->sum_money_,
                                (*iter).player_[1].sum_money_
                            );

                            (*iter).turn_player_ = &(*iter).player_[1];

                            turn_req_packet.set_my_money((*iter).player_[1].sum_money_);
                            turn_req_packet.set_opponent_money((*iter).player_[0].sum_money_);
                        }
                        else
                        {
                            log((*iter).room_key_)->info("-> submit_player:{}, sum_coin:{}, other_player_sum_coin:{}",
                                turn_type,
                                (*iter).turn_player_->key_,
                                (*iter).turn_player_->sum_money_,
                                (*iter).player_[0].sum_money_
                            );

                            (*iter).turn_player_ = &(*iter).player_[0];

                            turn_req_packet.set_my_money((*iter).player_[0].sum_money_);
                            turn_req_packet.set_opponent_money((*iter).player_[1].sum_money_);
                        }

                        (*iter).turn_player_->session_->handle_send(logic_server::PROCESS_TURN_REQ, turn_req_packet);
                    }

                }
            }
            break;

            case ROOM_INFO::END:
            {
                logic_server::packet_game_state_ntf game_state_packet;

                game_state_packet.set_state(2);

                if (iter->player_[0].remain_money_ != 0)
                    game_state_packet.set_win_player_key(iter->player_[0].key_);
                else
                    game_state_packet.set_win_player_key(iter->player_[1].key_);

                int disconnect_session = 0;

                for (int i = 0; i < 2; i++)
                {
                    if (!iter->player_[i].session_->is_connected())
                    {
                        disconnect_session++;
                        continue;
                    }

                    iter->player_[i].session_->handle_send(logic_server::GAME_STATE_NTF, game_state_packet);
                
                    iter->player_[i].session_->shut_down();
                }

                if (disconnect_session >= 2)
                {
                    if (iter->player_[0].remain_money_ != 0)
                    {
                        log((*iter).room_key_)->info("end_game, win_player_key:{}",
                            iter->player_[0].key_
                        );
                    }
                    else
                    {
                        log((*iter).room_key_)->info("end_game, win_player_key:{}",
                            iter->player_[1].key_
                        );
                    }

                    iter = room_list_.erase(iter);
                }
            }
            break;
            }
        }
    }
}
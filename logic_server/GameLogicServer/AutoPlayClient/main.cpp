#include "pre_header.h"
#include "logger.h"
#include "game_manager.h"
#include "network_manager.h"
#include "configurator.h"

int main(int argc, char* argv[])
{
    //int is_debug = 0;
    //configurator::get_value("is_debug_mode", is_debug);
    
    if (strcmp(argv[2], "true") == 0)
        logger::is_debug_mode(true);//(bool)is_debug);
    else
        logger::is_debug_mode(false);

    network_mgr->init_singleton();
    game_mgr->init_singleton();
    
    //logger::is_debug_mode(true);
    //game_mgr->play_game("1001", "1001");
    game_mgr->play_game(argv[1], argv[1]);


    game_mgr->release_singleton();
    network_mgr->release_singleton();

    return 0;
}
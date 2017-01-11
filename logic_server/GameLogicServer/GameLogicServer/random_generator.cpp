#include "random_generator.h"

std::default_random_engine random_generator::engine;

random_generator::random_generator()
{
}

random_generator::~random_generator()
{
}

int random_generator::get_random_int(int min, int max)
{
    std::uniform_int_distribution<int> random(min, max);

    return random(engine);
}
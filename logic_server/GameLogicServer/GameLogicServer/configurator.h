#pragma once
#include "preHeaders.h"

template<typename FROM_T, typename TO_T> struct is_castable
{
    typedef struct { char dummy[1]; } yes;
    typedef struct { char dummy[2]; } no;

    static yes check(TO_T);
    static no  check(...);

    enum { value = sizeof(check(*(FROM_T*)0)) == sizeof(yes) };
};

class configurator {
private:
    static json_spirit::mValue json_;

public:
    static bool get_value(std::string name, int& value)
    {
        if (json_.is_null())
        {
            std::ifstream is("config.json");

            if (!json_spirit::read(is, json_))
                return false;
        }

        json_spirit::mObject object = json_.get_obj();

        value = object.find(name)->second.get_int();

        return true;
    }

    static bool get_value(std::string name, std::string& value)
    {
        if (json_.is_null())
        {
            std::ifstream is("config.json");

            if (!json_spirit::read(is, json_))
                return false;
        }

        json_spirit::mObject object = json_.get_obj();

        value = object.find(name)->second.get_str();

        return true;
    }
};
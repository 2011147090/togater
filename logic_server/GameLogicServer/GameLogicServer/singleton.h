#pragma once
#include "critical_section.h"

template <typename T>
class singleton {
protected:
    static T* instance;

    singleton() {}

public:
    virtual ~singleton() {}

    static T* get_instance()
    {
        if (instance == NULL)
            instance = new T();

        return instance;
    }

    virtual bool init_singleton() = 0;
    virtual bool release_singleton() = 0;
};

template<typename T>
T* singleton<T>::instance = NULL;

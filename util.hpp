#ifndef _UTIL_H
#define _UTIL_H

#include <iostream>
#include <stack>
#include <vector>

template<typename T>
T pop_top(std::stack<T>& s)
{
    T t;

    if(s.size())
    {
        t = s.top();
        s.pop();
    }

    return t;
}

template<typename T>
T pop_back(std::vector<T>& v)
{
    if(v.size())
    {
        T t = v.back();
        v.pop_back();
        return t;
    }
    else
    {
        std::cerr << "INTERNAL: Attempted to pop_back empty vector" << std::endl;
        throw 0;
    }
}

bool is_integer(const char* str);
bool is_float(const char* str);
bool is_white(char input);

#endif

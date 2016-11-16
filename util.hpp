#ifndef _UTIL_H
#define _UTIL_H

#include <iostream>
#include <stack>
#include <queue>
#include <vector>

template<typename T>
T pop_front(std::queue<T>& q)
{
    T t;

    if(q.size())
    {
        t = q.front();
        q.pop();
    }

    return t;
}

template<typename T>
T pop_top(std::stack<T>& q)
{
    T t;

    if(q.size())
    {
        t = q.top();
        q.pop();
    }

    return t;
}

template<typename T>
T pop_back(std::vector<T>& q)
{
    if(q.size())
    {
        T t = q.back();
        q.pop_back();
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

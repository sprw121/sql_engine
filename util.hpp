#ifndef _UTIL_H
#define _UTIL_H

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
T pop_top(std::vector<T>& q)
{
    T t;

    if(q.size())
    {
        t = q.back();
        q.pop_back();
    }

    return t;
}

bool is_integer(const char* str);
bool is_float(const char* str);
bool is_white(char input);

#endif

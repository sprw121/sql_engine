#include <cstdlib>
#include <queue>

#include "util.hpp"


bool is_integer(const char* str)
{
    char* end;
    strtoll(str, &end, 10);
    return (*end == 0 && end != str);
}

bool is_float(const char* str)
{
    char* end;
    strtod(str, &end);
    return(*end == 0 && end != str);
}

bool is_white(char input)
{
    switch(input)
    {
        case ' ': case '\t': case '\n':
            return true;
        default: break;
    }

    return false;
}

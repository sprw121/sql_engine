#ifndef _EXIT_H
#define _EXIT_H

#include "query_object.hpp"

// Self - explanatory. Due to the parsing stage
// The only valid statements with exit in them is
// "exit".
struct exit_t : query_object
{
    void run() override
    {
        std::cerr << "Bye bye" << std::endl;
        exit(0);
    }
};

#endif

#ifndef _LIMIT_H
#define _LIMIT_H

#include <climits>

#include "../parser.hpp"

struct limit_t
{
    long long int limit;

    limit_t() : limit(LONG_MAX) {};
    limit_t(parse_tree_node node)
    {
        if(node.args.size() != 1)
        {
            std::cerr << "INTERNAL: Incorrect number of args to LIMIT." << std::endl;
            throw 0;
        }

        if(node.args[0].token.t != token_t::INT_LITERAL)
        {
            std::cerr << "Non-integer parameter passed to OFFSET." << std::endl;
            throw 0;
        }

        limit = node.args[0].token.value.i;
    }
};

#endif

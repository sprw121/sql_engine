#ifndef _OFFSET_H
#define _OFFSET_H

#include "../parser.hpp"

struct offset_t
{
    long long int offset;

    offset_t() : offset(0) {};
    offset_t(parse_tree_node node)
    {
        if(node.args.size() != 1)
        {
            std::cerr << "INTERNAL: Incorrect number of args to OFFSET." << std::endl;
            throw 0;
        }

        if(node.args[0].token.t != token_t::INT_LITERAL)
        {
            std::cerr << "Non-integer parameter passed to OFFSET." << std::endl;
            throw 0;
        }

        offset = node.args[0].token.value.i;
    }
};

#endif

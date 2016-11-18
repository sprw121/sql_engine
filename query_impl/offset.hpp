#ifndef _OFFSET_H
#define _OFFSET_H

#include "boost/variant.hpp"
#include "boost/variant/get.hpp"

#include "../parser.hpp"

struct offset_t
{
    long offset;

    offset_t() = default;
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

        offset = boost::get<long>(node.args[0].token.u);
    }
};

#endif
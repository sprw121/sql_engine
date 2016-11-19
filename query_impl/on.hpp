#ifndef _ON_H
#define _ON_H

#include "../parser.hpp"

struct on_t
{
    identitifer_t left_id;
    identitifer_t right_id;

    on_t() = default;
    on_t(parse_tree_node& node)
    {
        if(node.args.size() != 1)
        {
            std::cerr << "INTERNAL: Expected 2 args to ON." << std::endl;
            throw 0;
        }

        node = node.args[0];
        if(node.token.t != token_t::EQUAL)
        {
            std::cerr << "Expected EQUALS argument to ON." << std::endl;
            throw 0;
        }

        left_id = identitifer_t(node.args[0]);
        right_id = identitifer_t(node.args[1]);
    }
};

#endif

#ifndef _IDENTITIFER_H
#define _IDENTITIFER_H

#include <string>

#include "../parser.hpp"

struct identitifer_t
{
    std::string id;

    identitifer_t() = default;
    identitifer_t(parse_tree_node& node)
    {
        if(node.token.t != token_t::IDENTITIFER)
        {
            std::cerr << "INTERNAL: identitifer argument expected." << std::endl;
        }

        id = boost::get<std::string>(node.token.value);
    }
};

#endif

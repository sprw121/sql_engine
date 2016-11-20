#ifndef _AS_H
#define _AS_H

#include <string>

#include "../parser.hpp"
#include "../table.hpp"

#include "from.hpp"

template<typename T>
struct as_t
{
    std::string name;
    T value;

    as_t() = default;
    as_t(parse_tree_node& node)
    {
        if(node.args.size() != 2)
        {
            std::cerr << "INTERNAL: Expected 2 args to as clause." << std::endl;
        }

        name = boost::get<std::string>(node.args[1].token.value);
        value = T(node.args[0]);
    }

    as_t(parse_tree_node& node,
         table_map_t& tables)
    {
        if(node.args.size() != 2)
        {
            std::cerr << "INTERNAL: Expected 2 args to as clause." << std::endl;
        }

        name = boost::get<std::string>(node.args[1].token.value);
        value = T(node.args[0], tables);
    }

    as_t(parse_tree_node& node,
         from_t& from)
    {
        if(node.args.size() != 2)
        {
            std::cerr << "INTERNAL: Expected 2 args to as clause." << std::endl;
        }

        name = boost::get<std::string>(node.args[1].token.value);
        value = T(node.args[0], from);
    }
};

#endif

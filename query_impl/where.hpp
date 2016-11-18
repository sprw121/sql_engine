#ifndef _WHERE_H
#define _WHERE_H

#include <vector>

#include "../parser.hpp"
#include "../table.hpp"

#include "expression.hpp"

struct where_t
{
    std::vector<expression_t> filters;

    where_t() = default;
    where_t(parse_tree_node node,
            from_t& from)
    {
        if(node.args.size() == 0)
        {
            std::cerr << "No args supplied to WHERE clause." << std::endl;
            throw 0;
        }

        for(auto& arg : node.args)
        {
            filters.push_back(expression_t(arg, from));
        }
    }
};

#endif

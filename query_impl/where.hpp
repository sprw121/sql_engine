#ifndef _WHERE_H
#define _WHERE_H

#include <vector>

#include "../parser.hpp"
#include "../table.hpp"

#include "boolean_expression.hpp"

struct where_t
{
    std::vector<boolean_expression_t> filters;

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
            filters.emplace_back(boolean_expression_t(arg, from));
        }
    }

    bool filter()
    {
        for(auto& filter : filters)
            if(!filter.call()) return false;
        return true;
    }
};

#endif

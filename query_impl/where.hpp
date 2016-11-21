#ifndef _WHERE_H
#define _WHERE_H

#include <vector>

#include "../parser.hpp"
#include "../table.hpp"

#include "boolean_expression.hpp"

struct where_t
{
    std::vector<std::unique_ptr<boolean_expression_t>> filters;

    where_t() = default;
    where_t(parse_tree_node node,
            from_t& from) : filters()
    {
        // TODO We can potentially pass in an empty node here due
        // to the way there where's on handled in select.
        if(node.a_type != parse_tree_node::EMPTY)
        {
            if(node.args.size() == 0)
            {
                std::cerr << "No args supplied to WHERE clause." << std::endl;
                throw 0;
            }

            for(auto& arg : node.args)
                filters.emplace_back(boolean_factory(arg, from));
        }
    }

    bool filter()
    {
        for(auto& filter : filters)
            if(!filter->call()) return false;
        return true;
    }
};

#endif

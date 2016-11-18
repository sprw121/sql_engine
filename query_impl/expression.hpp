#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <memory>

#include "../parser.hpp"
#include "../table.hpp"
#include "../table_views.hpp"

#include "from.hpp"

struct expression_t
{
    std::shared_ptr<table_view> view;
    expression_t() = default;
    expression_t(parse_tree_node node,
                 from_t& from)
    {
        view = from.view;
    }

    cell call()
    {
        return view->access_column(0);
    }
};

#endif

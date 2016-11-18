struct from_t;

#ifndef _FROM_H
#define _FROM_H

#include <memory>

#include "../parser.hpp"
#include "../table.hpp"
#include "../table_views.hpp"

struct from_t
{
    std::shared_ptr<table_view> view;

    from_t() = default;
    from_t(parse_tree_node node, table_map_t& tables);
};

#endif

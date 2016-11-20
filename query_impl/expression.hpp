#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <memory>

#include "../parser.hpp"
#include "../table.hpp"
#include "../table_views.hpp"

#include "from.hpp"

struct expression_impl
{
    virtual cell call() = 0;
};

struct expression_t
{
    std::unique_ptr<expression_impl> impl;

    expression_t() = default;
    expression_t(expression_t&& e) : impl(std::move(e.impl)) { }
    expression_t& operator=(expression_t&& e) { impl = std::move(e.impl); return *this; }
    expression_t(parse_tree_node, from_t&);
    expression_t(std::string column_name, from_t& from);

    cell call();
};

#endif

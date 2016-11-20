#ifndef _BOOLEAN_EXPRESION_H
#define _BOOLEAN_EXPRESION_H

#include <memory>

#include "../parser.hpp"
#include "../table.hpp"
#include "../table_views.hpp"

#include "from.hpp"

struct boolean_expression_impl
{
    virtual bool call() = 0;
};

struct boolean_expression_t
{
    std::unique_ptr<boolean_expression_impl> impl;

    boolean_expression_t() = default;
    boolean_expression_t(boolean_expression_t&& e) : impl(std::move(e.impl)) { }
    boolean_expression_t& operator=(boolean_expression_t&& e)
    {
        impl = std::move(e.impl);
        return *this;
    }
    boolean_expression_t(parse_tree_node, from_t&);

    bool call();
};

#endif

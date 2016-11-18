#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <memory>

#include "../parser.hpp"
#include "../table.hpp"
#include "../table_views.hpp"

#include "from.hpp"

struct expression_impl
{
    std::shared_ptr<table_view> view;

    expression_impl(from_t& from)
    {
        view = from.view;
    }

    virtual cell call() = 0;
};

struct column_accessor : expression_impl
{
    int column;

    column_accessor(parse_tree_node node,
                    from_t& from) : expression_impl(from)
    {
        auto column_name = boost::get<std::string>(node.token.u);
        column = from.view->resolve_column(column_name);
    }

    cell call() override
    {
        return view->access_column(column);
    }
};

struct expression_t
{
    std::unique_ptr<expression_impl> impl;

    expression_t() = default;
    expression_t(expression_t&& e) : impl(std::move(e.impl)) {};
    expression_t& operator=(expression_t&& other) { return *this; }
    expression_t(parse_tree_node node,
                 from_t& from)
    {
        switch(node.token.t)
        {
            case token_t::IDENTITIFER:
                impl = std::unique_ptr<expression_impl>(new column_accessor(node, from));
                break;
        }
    }

    cell call()
    {
        return impl->call();
    }
};

#endif

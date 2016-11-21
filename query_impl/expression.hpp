#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <memory>

#include "../parser.hpp"
#include "../table.hpp"
#include "../table_views.hpp"

#include "from.hpp"

struct expression_t
{
    cell_type return_type;

    expression_t() = default;
    expression_t(parse_tree_node, from_t&);

    virtual cell call() = 0;
};

std::unique_ptr<expression_t> expression_factory(parse_tree_node, from_t&);
std::unique_ptr<expression_t> expression_factory(std::string column, from_t& from);

// Doing as<expression_t> would be incompatible,
// have a container type for that use case.
struct expression_container
{
    std::unique_ptr<expression_t> expression;

    expression_container() = default;
    expression_container(parse_tree_node node,
                         from_t& from)
    {
        expression = expression_factory(node, from);
    }
    expression_container(expression_container && e) : expression(std::move(e.expression)) { }
    expression_container& operator=(expression_container&& e)
    {
        expression = std::move(e.expression);
        return *this;
    }
};

#endif

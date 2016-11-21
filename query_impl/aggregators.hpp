#ifndef _AGGREGREGATORS_H
#define _AGGREGREGATORS_H

#include <memory>

#include "expression.hpp"
#include "from.hpp"
#include "../parser.hpp"

struct aggregator_t
{
    cell_type return_type;
    std::unique_ptr<expression_t> expr;

    aggregator_t() = default;
    aggregator_t(parse_tree_node node, from_t& from)
    {
        if(node.args.size() != 1)
        {
            std::cerr << "Only univariate aggregators supported." << std::endl;
            throw 0;
        }
        expr = expression_factory(node.args[0], from);
        return_type = expr->return_type;
    }

    virtual void accumulate() = 0;
    virtual cell call()       = 0;
};

std::unique_ptr<aggregator_t> aggregator_factory(parse_tree_node, from_t&);

// Doing as<expression_t> would be incompatible,
// have a container type for that use case.
struct aggregator_container
{
    std::unique_ptr<aggregator_t> aggregator;

    aggregator_container() = default;
    aggregator_container(parse_tree_node node,
                         from_t& from)
    {
        aggregator = aggregator_factory(node, from);
    }
    aggregator_container(aggregator_container && e) : aggregator(std::move(e.aggregator)) { }
    aggregator_container& operator=(aggregator_container&& e)
    {
        aggregator = std::move(e.aggregator);
        return *this;
    }
};

#endif

#ifndef _AGGREGREGATORS_H
#define _AGGREGREGATORS_H

#include <memory>

#include "expression.hpp"
#include "from.hpp"
#include "../parser.hpp"

// Implementation of aggregators (max, min, average, median)
// Abstract type with an accumulate method that is called
// for each row we iterate over, and a value method
// that should compute the aggregate value when we're done
// iterating.
struct aggregator_t
{
    cell_type return_type;
    std::unique_ptr<expression_t> expr;

    aggregator_t(std::unique_ptr<expression_t>& expr_) : expr(std::move(expr_))
    {
        return_type = expr->return_type;
    }

    virtual void accumulate() = 0;
    virtual cell value()      = 0;
    virtual ~aggregator_t()   = default;
};

std::unique_ptr<aggregator_t> aggregator_factory(parse_tree_node, from_t&);

// Doing as<aggregate_t> would be incompatible,
// have a container type that calls into aggregator_factory
// for that use case.
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

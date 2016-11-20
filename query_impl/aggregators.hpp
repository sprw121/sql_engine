#ifndef _AGGREGREGATORS_H
#define _AGGREGREGATORS_H

#include <memory>

#include "expression.hpp"
#include "from.hpp"
#include "../parser.hpp"

struct aggregator_impl
{
    expression_t expr;

    aggregator_impl(parse_tree_node node,
                    from_t& from)
    {
        if(node.args.size() != 1)
        {
            std::cerr << "Only univariate aggregators supported." << std::endl;
            throw 0;
        }
        expr = expression_t(node.args[0], from);
    }

    virtual void accumulate() = 0;
};

struct aggregator_t
{
    std::unique_ptr<aggregator_impl> impl;

    aggregator_t() = default;
    aggregator_t(aggregator_t&& e) : impl(std::move(e.impl)) { }
    aggregator_t& operator=(aggregator_t&& e) { impl = std::move(e.impl); return *this; }
    aggregator_t(parse_tree_node, from_t&);

    void accumulate()
    {
        impl->accumulate();
    }
};

#endif

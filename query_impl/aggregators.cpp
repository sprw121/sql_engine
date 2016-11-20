#include "../parser.hpp"
#include "../table.hpp"

#include "aggregators.hpp"
#include "expression_impl.hpp"
#include "boolean_expression_impl.hpp"

struct max_t : aggregator_impl
{
    cell max;
    max_t(parse_tree_node& node,
          from_t from) : aggregator_impl(node, from) {};

    void accumulate()
    {
    }

    cell value()
    {
        return max;
    }
};

struct min_t : aggregator_impl
{
    cell min;

    min_t(parse_tree_node& node,
          from_t from) : aggregator_impl(node, from) {};

    void accumulate()
    {
    }

    cell value()
    {
        return min;
    }
};

struct median_t : aggregator_impl
{
    std::vector<cell> vals;

    median_t(parse_tree_node& node,
             from_t from) : aggregator_impl(node, from) {};

    void accumulate()
    {
        vals.emplace_back(expr.call());
    }
};

struct average_t : aggregator_impl
{
    cell sum;

    average_t(parse_tree_node& node,
              from_t from) : aggregator_impl(node, from) {};

    void accumulate()
    {
    }

    cell value()
    {
    }
};

aggregator_t::aggregator_t(parse_tree_node node, from_t& from)
{
    if(boost::get<std::string>(node.token.value) == "max" ||
       boost::get<std::string>(node.token.value) == "MAX")
    {
        impl = std::unique_ptr<aggregator_impl>(new max_t(node, from));
    }
    else if(boost::get<std::string>(node.token.value) == "min" ||
            boost::get<std::string>(node.token.value) == "MIN")
    {
        impl = std::unique_ptr<aggregator_impl>(new max_t(node, from));
    }
    else if(boost::get<std::string>(node.token.value) == "median" ||
            boost::get<std::string>(node.token.value) == "MEDIAN")
    {
        impl = std::unique_ptr<aggregator_impl>(new max_t(node, from));
    }
    else if(boost::get<std::string>(node.token.value) == "average" ||
            boost::get<std::string>(node.token.value) == "AVERAGE")
    {
        impl = std::unique_ptr<aggregator_impl>(new max_t(node, from));
    }
    else
    {
        assert(0);
    }
}

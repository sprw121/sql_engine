#include <cassert>
#include <limits>

#include "../parser.hpp"
#include "../table.hpp"

#include "aggregators.hpp"

template<typename T>
struct max_t : aggregator_t
{
    T max;
    bool seen;

    max_t(parse_tree_node& node,
          from_t from) : aggregator_t(node, from)
    {
        seen = false;
        max  = std::numeric_limits<T>::lowest();
    }

    void accumulate()
    {
        seen = true;
        auto candidate = expr->call();
        if(*(T*)&candidate > max) max = *(T*)&candidate;
    }

    cell value()
    {
        if(!seen)
        {
            std::cerr << "Attempt to take max from empty tables." << std::endl;
            throw 0;
        }
        return *(cell*)&max;
    }
};

template<typename T>
struct min_t : aggregator_t
{
    T min;
    bool seen;

    min_t(parse_tree_node& node,
          from_t from) : aggregator_t(node, from)
    {
        seen = false;
        min = std::numeric_limits<T>::max();
    }

    void accumulate()
    {
        seen = true;
        auto candidate = expr->call();
        if(*(T*)&candidate < min) min = *(T*)&candidate;
    }

    cell value()
    {
        if(!seen)
        {
            std::cerr << "Attempt to take min from empty tables." << std::endl;
            throw 0;
        }
        std::cerr << "Taking min from empty tables." << std::endl;
        return *(cell*)&min;
    }
};

template<typename T>
struct median_t : aggregator_t
{
    std::vector<T> vals;
    unsigned long long int seen;

    median_t(parse_tree_node& node,
             from_t from) : aggregator_t(node, from)
    {
        vals.resize(from.view->height());
        seen = 0;
    }

    void accumulate()
    {
        auto val = expr->call();
        vals[seen++] = *(T*)&val;
    }

    cell value()
    {
        if(!seen)
        {
            std::cerr << "Attempt to take median from empty tables." << std::endl;
            throw 0;
        }

        return cell();
    }
};

template<typename T>
struct average_t : aggregator_t
{
    T sum;
    unsigned long long int seen;

    average_t(parse_tree_node& node,
              from_t from) : aggregator_t(node, from) {};

    void accumulate()
    {
        auto val = expr->call();
        sum += *(T*)&val;
    }

    cell value()
    {
        if(!seen)
        {
            std::cerr << "Attempt to take average from empty tables." << std::endl;
            throw 0;
        }

        auto av = sum / seen;
        return *(cell*)&av;
    }
};

std::unique_ptr<aggregator_t> aggregator_factory(parse_tree_node node, from_t& from)
{
/*    if(node.token.raw_rep == "max" ||
       node.token.raw_rep == "MAX")
    {
        impl = std::unique_ptr<aggregator_impl>(new max_t(node, from));
    }
    else if(node.token.raw_rep == "min" ||
            node.token.raw_rep == "MIN")
    {
        impl = std::unique_ptr<aggregator_impl>(new max_t(node, from));
    }
    else if(node.token.raw_rep == "median" ||
            node.token.raw_rep == "MEDIAN")
    {
        impl = std::unique_ptr<aggregator_impl>(new max_t(node, from));
    }
    else if(node.token.raw_rep == "average" ||
            node.token.raw_rep == "AVERAGE")
    {
        impl = std::unique_ptr<aggregator_impl>(new max_t(node, from));
    }
    else
    {
        assert(0);
    }*/
}

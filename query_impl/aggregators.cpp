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

    max_t(std::unique_ptr<expression_t>& expr_) : aggregator_t(expr_)
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

    min_t(std::unique_ptr<expression_t>& expr_) : aggregator_t(expr_)
    {
        seen = false;
        min  = std::numeric_limits<T>::max();
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
        return *(cell*)&min;
    }
};

// Implements quick select pivot based selection algorithm.
// We allocate up from all the space that might be necessary,
// although we may not fill them all as some views return a
// speculative maximum height.
template<typename T>
struct median_t : aggregator_t
{
    std::vector<T> vals;
    unsigned long long int seen;

    median_t(std::unique_ptr<expression_t>& expr_,
             from_t& from) : aggregator_t(expr_)
    {
        seen = 0;
        vals.reserve(from.view->height());
    }

    void accumulate()
    {
        auto val = expr->call();
        vals[seen++] = *(T*)&val;
    }

    // Partitions the underlying array around pivot,
    // such that all smaller elements come before
    // and larger elements come after.

    // Inclusion left and right, [left, right]
    unsigned int partition(unsigned int left, unsigned int right)
    {
        auto pivot     = vals[right];
        auto pivot_idx = right;
        right--;

        while(left < right)
        {
            // Swap if we can
            if(vals[left] >= pivot && vals[right] < pivot)
            {
                std::swap(vals[left], vals[right]);
                left++;
                right--;
            }
            // Skip those already in place
            if(vals[right] >= pivot)
                right--;
            if(vals[left] < pivot)
                left++;
        }

        // If we end up in the situation where left == right
        // they could have gotten there by both jumping
        // on the last loop iteration, therefore not processing
        // that element, so we have to check whether we can swap
        // pivot in here, or at the element to the right,
        // which has definitely been processed.
        // otherwise, left and right passed eachother,
        // and we know that left points at an element
        // that has been processed because right has
        // previously pointed at it.
        if(left == right && vals[left] <= pivot)
            std::swap(vals[++left], vals[pivot_idx]);
        else
            std::swap(vals[left], vals[pivot_idx]);
        return left;
    }

    // Process the range [left, right]
    T quick_select_impl(unsigned int left, unsigned int right, unsigned int kth)
    {
        // 1 element list
        if(right == left)
        {
            return vals[left];
        }

        // Partion the array, check where our pivot ended up.
        // If it ended on k, we're done, else recursive
        // go to either the left or right.
        auto pivot_idx = this->partition(left, right);
        if(pivot_idx == kth)
            return vals[pivot_idx];
        else if(pivot_idx > kth)
            return this->quick_select_impl(left, pivot_idx-1, kth);
        else if(pivot_idx < kth)
            return this->quick_select_impl(pivot_idx + 1, right, kth);
    }

    T quick_select()
    {
        return this->quick_select_impl(0, seen-1, seen/2);
    }

    cell value()
    {
        if(!seen)
        {
            std::cerr << "Attempt to take median from empty tables." << std::endl;
            throw 0;
        }
        auto val = this->quick_select();
        return *(cell*)&val;
    }
};

template<typename T>
struct average_t : aggregator_t
{
    T sum;
    unsigned long long int seen;

    average_t(std::unique_ptr<expression_t>& expr_) : aggregator_t(expr_)
    {
        sum = 0;
        seen = 0;
        return_type = FLOAT;
    }

    void accumulate()
    {
        auto val = expr->call();
        sum += *(T*)&val;
        seen++;
    }

    cell value()
    {
        if(!seen)
        {
            std::cerr << "Attempt to take average from empty tables." << std::endl;
            throw 0;
        }

        auto av = ((double)sum) / seen;
        return *(cell*)&av;
    }
};

// Here we first compile the expression that the aggregator aggregates,
// and construct the appropriate template.

// Constructor takes the expression and sets the return type.
std::unique_ptr<aggregator_t> aggregator_factory(parse_tree_node node, from_t& from)
{
    if(node.args.size() != 1)
    {
        std::cerr << "Only univariate aggregators supported." << std::endl;
        throw 0;
    }

    auto expr = expression_factory(node.args[0], from);
    auto return_type = expr->return_type;

    if(node.token.raw_rep == "max" ||
       node.token.raw_rep == "MAX")
    {
        if(return_type == cell_type::INT)
            return std::unique_ptr<aggregator_t>(
                new max_t<long long int>(expr));
        else
            return std::unique_ptr<aggregator_t>(
                new max_t<double>(expr));
    }
    else if(node.token.raw_rep == "min" ||
            node.token.raw_rep == "MIN")
    {
        if(return_type == cell_type::INT)
            return std::unique_ptr<aggregator_t>(
                new min_t<long long int>(expr));
        else
            return std::unique_ptr<aggregator_t>(
                new min_t<double>(expr));
    }
    else if(node.token.raw_rep == "median" ||
            node.token.raw_rep == "MEDIAN")
    {
        if(return_type == cell_type::INT)
            return std::unique_ptr<aggregator_t>(
                new median_t<long long int>(expr, from));
        else
            return std::unique_ptr<aggregator_t>(
                new median_t<double>(expr, from));
    }
    else if(node.token.raw_rep == "average" ||
            node.token.raw_rep == "AVERAGE")
    {
        if(return_type == cell_type::INT)
            return std::unique_ptr<aggregator_t>(
                new average_t<long long int>(expr));
        else
            return std::unique_ptr<aggregator_t>(
                new average_t<double>(expr));
    }
    else
    {
        assert(0);
    }
}

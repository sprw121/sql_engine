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

    unsigned int partition(unsigned int left, unsigned int right)
    {
        auto pivot     = vals[right - 1];
        auto pivot_idx = right - 1;

        while(left < right)
        {
            if(vals[left] >= pivot && vals[right] < pivot)
            {
                std::swap(vals[left], vals[right]);
                left++;
                right--;
            }
            if(vals[right] >= pivot)
                right--;
            if(vals[left] < pivot)
                left++;
        }

        std::swap(vals[left], vals[pivot_idx]);
        return left;
    }

    T quick_select_impl(unsigned int left, unsigned int right, unsigned int kth)
    {
        if(right == left + 1)
        {
            return vals[left];
        }

        auto pivot_idx = this->partition(left, right);
        if(pivot_idx = kth)
        {
            return vals[pivot_idx];
        }
        else if(pivot_idx > kth)
            return this->quick_select_impl(left, pivot_idx, kth);
        else if(pivot_idx < kth)
            return this->quick_select_impl(pivot_idx + 1, right, kth);
    }

    T quick_select()
    {
        if(seen % 2)
        {
            auto upper_median = this->quick_select_impl(0, seen, seen/2);
            T    lower_median = std::numeric_limits<T>::lowest();
            for(int i = 0; i < seen/2; i++)
                lower_median = vals[i] > lower_median ? vals[i] : lower_median;

            return (double)upper_median / 2  + (double)lower_median/2;
        }
        else
            return this->quick_select_impl(0, seen, seen/2);
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

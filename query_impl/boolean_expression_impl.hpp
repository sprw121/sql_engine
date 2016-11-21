#ifndef _EXPRESSION_IMPL_H
#define _EXPRESSION_IMPL_H

#include <cassert>
#include <cmath>
#include <limits>
#include <string>

#include "../parser.hpp"
#include "../table.hpp"

#include "boolean_expression.hpp"
#include "expression.hpp"
#include "from.hpp"

struct comparison_t : boolean_expression_t
{
    std::unique_ptr<expression_t> left;
    std::unique_ptr<expression_t> right;
    bool (*op)(const cell&, const cell&);

    comparison_t(parse_tree_node node,
                  from_t& from)
    {
        assert(node.args.size() == 2);

        left = expression_factory(node.args[0], from);
        right = expression_factory(node.args[1], from);
    }

    bool call() override
    {
        return (*op)(left->call(), right->call());
    }
};

bool int_equals(const cell& left, const cell& right)
{
    return *(long long int*)&left == *(long long int*)&right;
}

template<typename T, typename U>
bool fp_equals(const cell& left_, const cell& right_)
{
    double left  = (double)*(const T*)&left_;
    double right = (double)*(const T*)&left_;
    return (std::abs(left - right) <
                    std::numeric_limits<double>::epsilon() * std::abs(left + right) * 10);
}

struct eq_t : comparison_t
{
    eq_t(parse_tree_node node,
         from_t& from) : comparison_t(node, from)
    {
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::INT)
        {
            op = &int_equals;
        }
        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::INT)
        {
            op = &fp_equals<double, long long int>;
        }
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &fp_equals<long long int, double>;
            op = &int_equals;
        }
        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &fp_equals<double, double>;
        }
    }
};

bool int_nequals(const cell& left, const cell& right)
{
    return *(long long int*)&left != *(long long int*)&right;
}

template<typename T, typename U>
bool fp_nequals(const cell& left_, const cell& right_)
{
    double left  = (double)*(const T*)&left_;
    double right = (double)*(const U*)&right;
    return (std::abs(left - right) >=
                    std::numeric_limits<double>::epsilon() * std::abs(left + right) * 10);
}

struct neq_t : comparison_t
{
    neq_t(parse_tree_node node,
          from_t& from) : comparison_t(node, from)
    {
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::INT)
        {
            op = &int_nequals;
        }
        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::INT)
        {
            op = &fp_nequals<double, long long int>;
        }
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &fp_nequals<long long int, double>;
            op = &int_equals;
        }
        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &fp_nequals<double, double>;
        }
    }
};

template<typename T, typename U>
bool less_than(const cell& left, const cell& right)
{
    return *(const T*)&left < *(const U*)&right;
}

struct lt_t : comparison_t
{
    lt_t(parse_tree_node node,
          from_t& from) : comparison_t(node, from)
    {
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::INT)
        {
            op = &less_than<long long int, long long int>;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::INT)
        {
            op = &less_than<double, long long int>;
        }

        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &less_than<long long int , double>;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &less_than<double, double>;
        }
    }
};

template<typename T, typename U>
bool less_than_equals(const cell& left, const cell& right)
{
    return *(const T*)&left <= *(const U*)&right;
}

struct lteq_t : comparison_t
{
    lteq_t(parse_tree_node node,
           from_t& from) : comparison_t(node, from)
    {
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::INT)
        {
            op = &less_than_equals<long long int, long long int>;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::INT)
        {
            op = &less_than_equals<double, long long int>;
        }

        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &less_than_equals<long long int , double>;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &less_than_equals<double, double>;
        }
    }
};

template<typename T, typename U>
bool greater_than(const cell& left, const cell& right)
{
    return *(const T*)&left > *(const U*)&right;
}

struct gt_t : comparison_t
{
    gt_t(parse_tree_node node,
         from_t& from) : comparison_t(node, from)
    {
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::INT)
        {
            op = &greater_than<long long int, long long int>;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::INT)
        {
            op = &greater_than<double, long long int>;
        }

        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &greater_than<long long int , double>;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &greater_than<double, double>;
        }
    }
};

template<typename T, typename U>
bool greater_than_equals(const cell& left, const cell& right)
{
    return *(const T*)&left >= *(const U*)&right;
}

struct gteq_t : comparison_t
{
    gteq_t(parse_tree_node node,
           from_t& from) : comparison_t(node, from)
    {
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::INT)
        {
            op = &greater_than_equals<long long int, long long int>;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::INT)
        {
            op = &greater_than_equals<double, long long int>;
        }

        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &greater_than_equals<long long int , double>;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &greater_than_equals<double, double>;
        }
    }
};

struct not_op : boolean_expression_t
{
    std::unique_ptr<boolean_expression_t> operand;

    not_op(parse_tree_node node,
           from_t& from)
    {
        operand = boolean_factory(node.args[0], from);
    }

    bool call() override
    {
        return !operand->call();
    }
};

struct and_t
{
    bool operator()(const bool& left, const bool& right)
    {
        return left && right;
    }
};

struct or_t
{
    bool operator()(const bool& left, const bool& right)
    {
        return left || right;
    }
};

template<typename T>
struct logical_op : boolean_expression_t
{
    std::unique_ptr<boolean_expression_t> left;
    std::unique_ptr<boolean_expression_t> right;

    logical_op(parse_tree_node node,
               from_t& from)
    {
        assert(node.args.size() == 2);

        left = boolean_factory(node.args[0], from);
        right = boolean_factory(node.args[1], from);
    }

    bool call() override
    {
        return T()(left->call(), right->call());
    }
};

#endif

#ifndef _EXPRESSION_IMPL_H
#define _EXPRESSION_IMPL_H

#include <cmath>
#include <string>

#include "boost/variant/apply_visitor.hpp"

#include "../parser.hpp"
#include "../table.hpp"

#include "boolean_expression.hpp"
#include "expression.hpp"
#include "from.hpp"

struct eq_t : public boost::static_visitor<bool>
{
    bool operator()(long long int& left, long long int& right) const
    {
        return left == right;
    }

    bool operator()(double& left, double& right) const
    {
        return (std::abs(left - right) <
                std::numeric_limits<double>::epsilon() * std::abs(left + right) * 3)
                || std::abs(left - right) < std::numeric_limits<double>::min();
    }

    bool operator()(std::string& left, std::string& right) const
    {
        return left == right;
    }

    template<class T, class U>
    bool operator()(T& left, U& right) const
    {
        std::cerr << std::endl << std::endl
                  << "**** Attempting to compare types ****" << std::endl;
        throw 0;
    }
};

struct neq_t : public boost::static_visitor<bool>
{
    bool operator()(long long int& left, long long int& right) const
    {
        return left != right;
    }

    bool operator()(double& left, double& right) const
    {
        return (std::abs(left - right) >=
                std::numeric_limits<double>::epsilon() * std::abs(left + right) * 3)
                && std::abs(left - right) >= std::numeric_limits<double>::min();
    }

    bool operator()(std::string& left, std::string& right) const
    {
        return left != right;
    }

    template<class T, class U>
    bool operator()(T& left, U& right) const
    {
        std::cerr << std::endl << std::endl
                  << "**** Attempting to compare types ****" << std::endl;
    }
};

struct lt_t : public boost::static_visitor<bool>
{
    bool operator()(long long int& left, long long int& right) const
    {
        return left < right;
    }

    bool operator()(double& left, double& right) const
    {
        return left < right;
    }

    template<class T, class U>
    bool operator()(T& left, U& right) const
    {
        std::cerr << std::endl << std::endl
                  << "**** Attempting to compare types ****" << std::endl;
    }
};

struct lteq_t : public boost::static_visitor<bool>
{
    bool operator()(long long int& left, long long int& right) const
    {
        return left <= right;
    }

    bool operator()(double& left, double& right) const
    {
        return left <= right;
    }

    template<class T, class U>
    bool operator()(T& left, U& right) const
    {
        std::cerr << std::endl << std::endl
                  << "**** Attempting to compare types ****" << std::endl;
    }
};

struct gt_t : public boost::static_visitor<bool>
{
    bool operator()(long long int& left, long long int& right) const
    {
        return left > right;
    }

    bool operator()(double& left, double& right) const
    {
        return left > right;
    }

    template<class T, class U>
    bool operator()(T& left, U& right) const
    {
        std::cerr << std::endl << std::endl
                  << "**** Attempting to compare types ****" << std::endl;
    }
};

struct gteq_t : public boost::static_visitor<bool>
{
    bool operator()(long long int& left, long long int& right) const
    {
        return left >= right;
    }

    bool operator()(double& left, double& right) const
    {
        return left >= right;
    }

    template<class T, class U>
    bool operator()(T& left, U& right) const
    {
        std::cerr << std::endl << std::endl
                  << "**** Attempting to compare types ****" << std::endl;
    }
};

struct and_t
{
    bool operator()(bool left, bool right)
    {
        return left && right;
    }
};

struct or_t
{
    bool operator()(bool left, bool right)
    {
        return left || right;
    }
};

template<typename T>
struct logical_op : boolean_expression_impl
{
    boolean_expression_t left;
    boolean_expression_t right;

    logical_op(parse_tree_node node,
               from_t& from)
    {
        left = boolean_expression_t(node.args[0], from);
        right = boolean_expression_t(node.args[1], from);
    }

    bool call() override
    {
        return T()(left.call(), right.call());
    }
};

template<typename T>
struct comparison_op : boolean_expression_impl
{
    expression_t left;
    expression_t right;

    comparison_op(parse_tree_node node,
                  from_t& from)
    {
        left = expression_t(node.args[0], from);
        right = expression_t(node.args[1], from);
    }

    bool call() override
    {
        auto left_arg = left.call();
        auto right_arg = right.call();
        return boost::apply_visitor(T(), left_arg, right_arg);
    }
};

struct not_op : boolean_expression_impl
{
    boolean_expression_t operand;

    not_op(parse_tree_node node,
           from_t& from)
    {
        operand = boolean_expression_t(node.args[0], from);
    }

    bool call() override
    {
        return !operand.call();
    }
};
#endif

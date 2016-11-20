#ifndef _EXPRESSION_IMPL_H
#define _EXPRESSION_IMPL_H

#include <cmath>
#include <string>

#include "boost/variant/apply_visitor.hpp"

#include "../parser.hpp"
#include "../table.hpp"

#include "expression.hpp"
#include "from.hpp"

struct add_t : public boost::static_visitor<cell>
{
    cell operator()(long long int& left, long long int& right) const
    {
        return left + right;
    }

    cell operator()(double& left, double& right) const
    {
        return left + right;
    }

    cell operator()(double& left, long long int& right) const
    {
        return left + right;
    }

    cell operator()(long long int& left, double& right) const
    {
        return left + right;
    }

    cell operator()(std::string& left, std::string& right) const
    {
        return left + right;
    }

    template<class T, class U>
    cell operator()(T& left, U& right) const
    {
        return cell();
    }
};

struct sub_t : public boost::static_visitor<cell>
{
    cell operator()(long long int& left, long long int& right) const
    {
        return left - right;
    }

    cell operator()(double& left, double& right) const
    {
        return left - right;
    }

    cell operator()(double& left, long long int& right) const
    {
        return left - right;
    }

    cell operator()(long long int& left, double& right) const
    {
        return left - right;
    }

    template<class T, class U>
    cell operator()(T& left, U& right) const
    {
        return cell();
    }
};

struct mult_t : public boost::static_visitor<cell>
{
    cell operator()(long long int& left, long long int& right) const
    {
        return left * right;
    }

    cell operator()(double& left, double& right) const
    {
        return left * right;
    }

    cell operator()(double& left, long long int& right) const
    {
        return left * right;
    }

    cell operator()(long long int& left, double& right) const
    {
        return left * right;
    }

    template<class T, class U>
    cell operator()(T& left, U& right) const
    {
        return cell();
    }
};

// div_t reserved
struct divi_t : public boost::static_visitor<cell>
{
    cell operator()(long long int& left, long long int& right) const
    {
        return (double)left / right;
    }

    cell operator()(double& left, double& right) const
    {
        return left / right;
    }

    cell operator()(double& left, long long int& right) const
    {
        return left / right;
    }

    cell operator()(long long int& left, double& right) const
    {
        return left / right;
    }

    template<class T, class U>
    cell operator()(T& left, U& right) const
    {
        return cell();
    }
};

struct mod_t : public boost::static_visitor<cell>
{
    cell operator()(long long int& left, long long int& right) const
    {
        return left % right;
    }

    template<class T, class U>
    cell operator()(T& left, U& right) const
    {
        return cell();
    }
};

struct exp_t : public boost::static_visitor<cell>
{
    cell operator()(long long int& left, long long int& right) const
    {
        return pow(left, right);
    }

    cell operator()(double& left, double& right) const
    {
        return pow(left, right);
    }

    cell operator()(double& left, long long int& right) const
    {
        return pow(left, right);
    }

    cell operator()(long long int& left, double& right) const
    {
        return pow(left, right);
    }

    template<class T, class U>
    cell operator()(T& left, U& right) const
    {
        return cell();
    }
};

template<typename T>
struct binary_op : expression_impl
{
    expression_t left;
    expression_t right;

    binary_op(parse_tree_node node,
              from_t& from)
    {
        left = expression_t(node.args[0], from);
        right = expression_t(node.args[1], from);
    }

    cell call() override
    {
        auto left_arg = left.call();
        auto right_arg = right.call();
        return boost::apply_visitor(T(), left_arg, right_arg);
    }
};

struct column_accessor : expression_impl
{
    int column;
    std::shared_ptr<table_view> view;

    column_accessor(parse_tree_node node,
                    from_t& from)
    {
        auto column_name = boost::get<std::string>(node.token.value);
        column = from.view->resolve_column(column_name);
        view = from.view;
    }

    column_accessor(std::string column_name,
                    from_t& from)
    {
        column = from.view->resolve_column(column_name);
        view = from.view;
    }

    cell call() override
    {
        return view->access_column(column);
    }
};

struct const_expr : expression_impl
{
    cell value;

    const_expr(parse_tree_node node,
               from_t& from)
    {
        if(node.token.t == token_t::INT_LITERAL)
            value = boost::get<long long int>(node.token.value);
        else if(node.token.t == token_t::FLOAT_LITERAL)
            value = boost::get<double>(node.token.value);
        else if(node.token.t == token_t::STR_LITERAL)
            value = boost::get<std::string>(node.token.value);
        else
        {
            std::cerr << "INTERNAL: No literal passed to const_expr: "
                      << output_token(node.token) << std::endl;
            throw 0;
        }
    }

    cell call() override
    {
        return value;
    }
};

#endif

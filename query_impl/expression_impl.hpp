#ifndef _EXPRESSION_IMPL_H
#define _EXPRESSION_IMPL_H

#include <cassert>
#include <cmath>
#include <string>

#include "../parser.hpp"
#include "../table.hpp"

#include "identitifer.hpp"
#include "expression.hpp"
#include "from.hpp"

// Binary operations derived from here,
// and set there appropriate op function
// based on the return types of the left
// and right expressions.
struct binary_op : expression_t
{
    std::unique_ptr<expression_t> left;
    std::unique_ptr<expression_t> right;
    cell (*op)(const cell&, const cell&);

    binary_op(parse_tree_node node,
              from_t& from)
    {
        assert(node.args.size() == 2);

        left  = expression_factory(node.args[0], from);
        right = expression_factory(node.args[1], from);
    }

    cell call() override
    {
        return (*op)(left->call(), right->call());
    }
};

// Unsafe way of implementing operations, but efficient,
// and allows us to generate code from templates
template<typename T, typename U>
cell add(const cell& left, const cell& right)
{
    return *(const T*)&left + *(const U*)&right;
}

struct add_t : binary_op
{
    add_t(parse_tree_node node,
          from_t& from) : binary_op(node, from)
    {
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::INT)
        {
            op = &add<long long int, long long int>;
            return_type = cell_type::INT;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::INT)
        {
            op = &add<double, long long int>;
            return_type = cell_type::FLOAT;
        }

        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &add<long long int , double>;
            return_type = cell_type::FLOAT;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &add<double, double>;
            return_type = cell_type::FLOAT;
        }
    }
};

template<typename T, typename U>
cell subtract(const cell& left, const cell& right)
{
    return *(const T*)&left - *(const U*)&right;
}

struct sub_t : binary_op
{
    sub_t(parse_tree_node node,
          from_t& from) : binary_op(node, from)
    {
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::INT)
        {
            op = &subtract<long long int, long long int>;
            return_type = cell_type::INT;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::INT)
        {
            op = &subtract<double, long long int>;
            return_type = cell_type::FLOAT;
        }

        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &subtract<long long int , double>;
            return_type = cell_type::FLOAT;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &subtract<double, double>;
            return_type = cell_type::FLOAT;
        }
    }
};

template<typename T, typename U>
cell multiply(const cell& left, const cell& right)
{
    return (*(const T*)&left) * (*(const U*)&right);
}

struct mult_t : binary_op
{
    mult_t(parse_tree_node node,
           from_t& from) : binary_op(node, from)
    {
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::INT)
        {
            op = &multiply<long long int, long long int>;
            return_type = cell_type::INT;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::INT)
        {
            op = &multiply<double, long long int>;
            return_type = cell_type::FLOAT;
        }

        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &multiply<long long int , double>;
            return_type = cell_type::FLOAT;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &multiply<double, double>;
            return_type = cell_type::FLOAT;
        }
    }
};

template<typename T, typename U>
cell divide(const cell& left, const cell& right)
{
    return (double)*(const T*)&left / *(const U*)&right;
}

struct divi_t : binary_op
{
    divi_t(parse_tree_node node,
           from_t& from) : binary_op(node, from)
    {
        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::INT)
        {
            op = &divide<long long int, long long int>;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::INT)
        {
            op = &divide<double, long long int>;
        }

        if(left->return_type == cell_type::INT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &divide<long long int , double>;
        }

        if(left->return_type == cell_type::FLOAT &&
           right->return_type == cell_type::FLOAT)
        {
            op = &divide<double, double>;
        }

        return_type = cell_type::FLOAT;
    }
};

cell modu(const cell& left, const cell& right)
{
    return *(long long int*)&left % *(long long int*)&right;
}

struct mod_t : binary_op
{
    mod_t(parse_tree_node node,
          from_t& from) : binary_op(node, from)
    {
        if(left->return_type != cell_type::INT)
        {
            std::cerr << "Left arg to MOD must be of expression of type INT."
                      << std::endl;
        }

        if(right->return_type != cell_type::INT)
        {
            std::cerr << "Right arg to MOD must be of expression of type INT."
                      << std::endl;
        }

        op = &modu;
    }
};

template<typename T>
cell negate(const cell& operand)
{
    return *(T*)&operand * -1;
}

struct negate_t : expression_t
{
    std::unique_ptr<expression_t> operand;
    cell (*op)(const cell&);

    negate_t(parse_tree_node node,
             from_t& from)
    {
        if(operand->return_type == cell_type::INT)
        {
            op = &negate<long long int>;
        }
        if(operand->return_type == cell_type::FLOAT)
        {
            op = &negate<double>;
        }
        return_type = operand->return_type;
    }

    cell call() override
    {
        return (*op)(operand->call());
    }
};

// Leaf node to access a column in a view.
struct column_accessor : expression_t
{
    int column;
    std::shared_ptr<table_view> view;

    column_accessor(parse_tree_node node,
                    from_t& from)
    {
        identitifer_t identitifer(node);
        view        = from.view;
        column      = view->resolve_column(identitifer.id);
        return_type = view->column_types[column];
    }

    column_accessor(std::string column_name,
                    from_t& from)
    {
        view        = from.view;
        column      = view->resolve_column(column_name);
        return_type = view->column_types[column];
    }

    cell call() override
    {
        return view->access_column(column);
    }
};

// Leaf node that emits a constant value
struct const_expr : expression_t
{
    cell value;

    const_expr(parse_tree_node& node,
               from_t& from)
    {
        if(node.token.t == token_t::STR_LITERAL)
        {
            std::cerr << "String literals not supported." << std::endl;
            throw 0;
        }

        assert(node.token.t == token_t::INT_LITERAL ||
               node.token.t == token_t::FLOAT_LITERAL);

        value = node.token.value;
        if(node.token.t == token_t::INT_LITERAL)
            return_type = cell_type::INT;
        if(node.token.t == token_t::FLOAT_LITERAL)
            return_type = cell_type::FLOAT;
    }

    cell call() override
    {
        return value;
    }
};

#endif

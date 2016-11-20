#include <iomanip>
#include <iostream>
#include <sstream>
#include <stack>

#include "select.hpp"
#include "../util.hpp"

select_t::select_t(parse_tree_node& node,
       table_map_t& tables)
{
    if(node.args.size() < 2)
    {
        std::cerr << "Not enough args to select." << std::endl;
        throw 0;
    }

    bool seen_offset = false, seen_limit = false, seen_where = false;
    parse_tree_node where_node;
    unsigned int i = 0;
    for(auto& arg : node.args)
    {
        switch(arg.token.t)
        {
            case token_t::OFFSET:
            {
                if(seen_offset || seen_where || seen_limit)
                {
                    std::cerr << "Unexpected OFFSET clause." << std::endl;
                    throw 0;
                }
                offset = offset_t(arg);
                seen_offset = true;
                break;
            }
            case token_t::LIMIT:
            {
                if(seen_limit || seen_where)
                {
                    std::cerr << "Unexpected LIMIT clause." << std::endl;
                    throw 0;
                }
                limit = limit_t(arg);
                seen_limit = true;
                break;
            }
            case token_t::WHERE:
            {
                if(seen_where)
                {
                    std::cerr << "Unexpected WHERE clause." << std::endl;
                    throw 0;
                }
                // Defer processing of where until we have a FROM.
                where_node = arg;
                seen_where = true;
                break;
            }
            case token_t::FROM:
            {
                from = from_t(arg, tables);
                i++;
                goto end_loop;
            }
            default:
            {
                std::cerr << "Unexpected clause after FROM." << std::endl;
                throw 0;
            }
        }
    }
    end_loop:
    if(i == node.args.size())
    {
        std::cerr << "No FROM clause seen." << std::endl;
    }
    if(seen_offset && !seen_limit)
    {
        std::cerr << "OFFSET clause without LIMIT clause." << std::endl;
        throw 0;
    }

    if(seen_where)
    {
        where = where_t(where_node, from);
    }

    std::stack<parse_tree_node> expression_stack;
    while(i < node.args.size())
    {
        expression_stack.push(node.args[i++]);
    }

    int column_idx = 0;
    while(!expression_stack.empty())
    {
        parse_tree_node arg = pop_top(expression_stack);
        std::cout << arg.token.value << std::endl;
        if(arg.token.t == token_t::AS)
        {
            as_t<expression_t> named_expression(arg, from);
            column_names.push_back(named_expression.name);
            columns.push_back(std::move(named_expression.value));
        }
        else if (arg.token.t == token_t::IDENTITIFER)
        {
            column_names.push_back(boost::get<std::string>(arg.token.value));
            columns.push_back(expression_t(arg, from));
        }
        else if (arg.token.t == token_t::SELECT_ALL)
        {
            for(auto& column_name : from.view->column_names)
            {
                column_names.push_back(column_name);
                columns.push_back(expression_t(column_name, from));
            }
            column_idx += column_names.size();
        }
        else
        {
            std::stringstream stream;
            stream << "col_" << column_idx;
            columns.push_back(expression_t(arg, from));
            column_names.push_back(stream.str());
        }
        column_idx++;
    }
}

cell select_t::access_column(unsigned int i)
{
    return columns[i].call();
}

void select_t::advance_row()
{
    from.view->advance_row();
}

bool select_t::empty()
{
    return from.view->empty();
}

unsigned int select_t::width()
{
    return columns.size();
}

unsigned int select_t::height()
{
    return from.view->height();
}

void select_t::run()
{
    for(auto& col_name : column_names)
        std::cout << std::setw(15) << col_name << " | ";
    std::cout << std::endl;
    for(unsigned int i = 0 ; i < width(); i++)
        std::cout << "----------------+-";
    std::cout << std::endl;
    while(!empty())
    {
        for(auto& column: columns)
        {
            std::cout << std::setw(15) << column.call() << " | ";
        }
        std::cout << std::endl;
        advance_row();
    }
}

std::shared_ptr<table_iterator> select_t::load()
{
    return std::make_shared<table_iterator>(*this);
}

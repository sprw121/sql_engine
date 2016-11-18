#ifndef _COMPILE_H
#define _COMPILE_H

#include <algorithm>
#include <climits>
#include <iostream>
#include <sstream>
#include <memory>
#include <unordered_map>

#include "lexer.hpp"
#include "parser.hpp"
#include "table.hpp"
#include "table_views.hpp"


// This file contains all the code necessary to compile
// a query parse tree into an executable query.

// The general pattern here is for each token to to
// Have it's own struct, and the constructor of that
// struct takes the associated node in the parse_tree
// to construct itself.

// The table_name -> table map from inside the engine
// is also passed around in other to resolve identitifers
// where necessary.

// Commands are implemented as a abstract class type that
// must implement a run method (EXIT, SELECT, DESCRIBE, SHOW, LOAD).

// Certain types (JOIN, SELECT) also implement the table_view
// interface.

struct query_object
{
    virtual void run() { }
};

struct identitifer_t
{
    std::string id;

    identitifer_t() = default;
    identitifer_t(parse_tree_node& node)
    {
        if(node.token.t != token_t::IDENTITIFER)
        {
            std::cerr << "Table id argument expected." << std::endl;
        }

        id = boost::get<std::string>(node.token.u);
    }
};

template<typename T>
struct as_t
{
    std::string name;
    T value;

    as_t() = default;
    as_t(parse_tree_node& node)
    {
        if(node.args.size() != 2)
        {
            std::cerr << "INTERNAL: Expected 2 args to as clause." << std::endl;
        }

        name = boost::get<std::string>(node.args[1].token.u);
        value = T(node.args[0]);
    }

    as_t(parse_tree_node& node,
         table_map_t& tables)
    {
        if(node.args.size() != 2)
        {
            std::cerr << "INTERNAL: Expected 2 args to as clause." << std::endl;
        }

        name = boost::get<std::string>(node.args[1].token.u);
        value = T(node.args[0], tables);
    }
};

struct expression_t
{
    expression_t() = default;
    expression_t(parse_tree_node node,
                 table_map_t& tables)
    {
    }
};

struct offset_t
{
    long offset;

    offset_t() = default;
    offset_t(parse_tree_node node)
    {
        if(node.args.size() != 1)
        {
            std::cerr << "INTERNAL: Incorrect number of args to OFFSET." << std::endl;
            throw 0;
        }

        if(node.args[0].token.t != token_t::INT_LITERAL)
        {
            std::cerr << "Non-integer parameter passed to OFFSET." << std::endl;
            throw 0;
        }

        offset = boost::get<long>(node.args[0].token.u);
    }
};

struct limit_t
{
    long limit;

    limit_t() : limit(LONG_MAX) {};
    limit_t(parse_tree_node node)
    {
        if(node.args.size() != 1)
        {
            std::cerr << "INTERNAL: Incorrect number of args to LIMIT." << std::endl;
            throw 0;
        }

        if(node.args[0].token.t != token_t::INT_LITERAL)
        {
            std::cerr << "Non-integer parameter passed to OFFSET." << std::endl;
            throw 0;
        }

        limit = boost::get<long>(node.args[0].token.u);
    }
};

struct where_t
{
    std::vector<expression_t> filters;

    where_t() = default;
    where_t(parse_tree_node node,
            table_map_t& tables)
    {
        if(node.args.size() == 0)
        {
            std::cerr << "No args supplied to WHERE clause." << std::endl;
            throw 0;
        }

        for(auto& arg : node.args)
        {
            filters.push_back(expression_t(arg, tables));
        }
    }
};

struct select_t;

struct from_t
{
    table_view* view;

    from_t() = default;
    from_t(parse_tree_node node,
           table_map_t& tables)
    {
        if(!node.args.size())
        {
            std::cerr << "INTERNAL: No args to FROM." << std::endl;
            throw 0;
        }
        if(node.args.size() != 1)
        {
            std::cerr << "Variadic FROM not supported (1 arg required)" << std::endl;
            throw 0;
        }

        auto& arg = node.args[0];
        if(arg.token.t == token_t::AS)
        {
            as_t<select_t> named_select(arg, tables);
        }
        else if(arg.token.t == token_t::IDENTITIFER)
        {
        }
        else if(arg.token.t == token_t::JOIN)
        {
        }
    }
};

struct exit_t : query_object
{
    void run() override
    {
        std::cerr << "Bye bye" << std::endl;
        exit(0);
    }
};

struct describe_t : query_object
{
    table_map_t* tables;
    std::vector<identitifer_t> table_identitifers;

    describe_t() = default;
    describe_t(parse_tree_node node,
             table_map_t& tables_)
    {
        if(!node.args.size())
        {
            std::cerr << "INTERNAL: No args to DESCRIBE." << std::endl;
            throw 0;
        }

        for(auto& arg : node.args)
        {
            table_identitifers.push_back(identitifer_t(arg));
        }
    }

    void run() override
    {
        for(auto& t : table_identitifers)
        {
            auto table = tables->find(t.id);
            if(table != tables->end())
            {
                table->second.describe();
            }
            else
            {
                std::cerr << "Could not resolve table name " << t.id << "." << std::endl;
                throw 0;
            }
        }

    }
};

struct show_t : query_object
{
    table_map_t* tables;

    show_t() = default;
    show_t(parse_tree_node& node,
         table_map_t& tables_)
    {
        if(!node.args.size())
        {
            std::cerr << "INTERNAL: No args to SHOW." << std::endl;
            throw 0;
        }

        if(node.args.size() != 1 || node.args[0].token.t != token_t::TABLES)
        {
            std::cerr << "Only SHOW TABLES implemented." << std::endl;
            throw 0;
        }

        tables = &tables_;
    }

    void run() override
    {
        std::cout << std::endl;
        if(tables->size() == 0)
        {
            std::cout << "No tables loaded." << std::endl;
        }
        else
        {
            std::cout << "TABLE NAMES" << std::endl
                      << "-----------" << std::endl;
        }
        for(auto& table: *tables)
        {
            std::cout << table.first << std::endl;
        }
        std::cout << std::endl;
    }

};

struct load_t : query_object
{
    table_map_t* tables;
    std::vector<as_t<identitifer_t>> load_args;

    load_t() = default;
    load_t(parse_tree_node& node,
           table_map_t& tables_)
    {
        if(!node.args.size())
        {
            std::cerr << "INTERNAL: No args to SHOW." << std::endl;
            throw 0;
        }

        for(auto& arg : node.args)
        {
            if(arg.args.size() != 2)
            {
                std::cerr << "INTERNAL: Not enough args to AS." << std::endl;
                throw 0;
            }

            load_args.push_back(as_t<identitifer_t>(arg));
        }
        tables = &tables_;
    }

    void run() override
    {
        for(auto& load_arg : load_args)
        {
            auto csv = load_arg.value.id;
            auto table_name = load_arg.name;

            if(tables->find(table_name) != tables->end())
            {
                std::cerr << "Invalid input: Attempted to load more than one"
                          << "table of the same name.    " << table_name << std::endl;
                throw 0;
            }

            tables->emplace(std::make_pair(table_name, table(csv)));
        }
    }
};

struct select_t : query_object, table_view
{
    std::vector<expression_t> columns;
    std::vector<std::string> column_names;
    std::vector<type> column_types;
    from_t from;
    where_t where;
    limit_t limit;
    offset_t offset;

    select_t() = default;
    select_t(parse_tree_node& node,
           table_map_t& tables)
    {
        if(node.args.size() < 2)
        {
            std::cerr << "Not enough args to select." << std::endl;
            throw 0;
        }

        bool seen_offset = false, seen_limit = false, seen_where = false;
        int i = 0;
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
                        std::cerr << "Unexpected OFFSET clause." << std::endl;
                        throw 0;
                    }
                    limit = limit_t(arg);
                    break;
                }
                case token_t::WHERE:
                {
                    if(seen_where)
                    {
                        std::cerr << "Unexpected OFFSET clause." << std::endl;
                        throw 0;
                    }
                    where = where_t(arg, tables);
                    seen_where = true;
                    break;
                }
                case token_t::FROM:
                {
                    from = from_t(arg, tables);
                    goto end_loop;
                }
                default:
                {
                    std::cerr << "Unexpected clause after FROM." << std::endl;
                    break;
                }
            }
            i++;
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
        for(; i < node.args.size(); i++)
        {
            if(node.token.t == token_t::AS)
            {
                as_t<expression_t> named_expression(node, tables);
                column_names.push_back(named_expression.name);
                columns.push_back(named_expression.value);
            }
            else
            {
                std::stringstream stream;
                stream << "col_" << i;
                columns.push_back(expression_t(node, tables));
                column_names.push_back(stream.str());
            }
        }
    }

    cell access_column(unsigned int i) override
    {
    }

    void advance_row() override
    {
    }

    bool empty() override
    {
    }

    unsigned int width() override
    {
    }

    unsigned int height() override
    {
    }
};

std::unique_ptr<query_object> compile_query(parse_tree_node& node,
                                            table_map_t& tables)
{
    switch(node.token.t)
    {
        case token_t::DESCRIBE:
        {
            return std::unique_ptr<query_object>(new describe_t(node, tables));
        }
        case token_t::EXIT:
        {
            return std::unique_ptr<query_object>(new exit_t());
        }
        case token_t::SHOW:
        {
            return std::unique_ptr<query_object>(new show_t(node, tables));
        }
        case token_t::LOAD:
        {
            return std::unique_ptr<query_object>(new load_t(node, tables));
        }
        case token_t::SELECT:
        {
            return std::unique_ptr<query_object>(new load_t(node, tables));
        }
    }

    std::cerr << "No command seen in query." << std::endl;
};

#endif

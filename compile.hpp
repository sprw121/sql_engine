#ifndef _COMPILE_H
#define _COMPILE_H

#include <iostream>
#include <memory>
#include <unordered_map>

#include "lexer.hpp"
#include "parser.hpp"
#include "table.hpp"
#include "table_views.hpp"

struct identitifer
{
    std::string id;

    identitifer() : id() {};
    identitifer(parse_tree_node& node)
    {
        if(node.token.t != token_t::IDENTITIFER)
        {
            std::cerr << "Table id argument expected." << std::endl;
        }

        id = boost::get<std::string>(node.token.u);
    }
};

template<typename T>
struct as
{
    std::string name;
    T value;

    as(parse_tree_node& node)
    {
        if(node.args.size() != 2)
        {
            std::cerr << "INTERNAL: Expected 2 args to as clause." << std::endl;
        }

        name = boost::get<std::string>(node.args[1].token.u);

        value =  T(node.args[0]);
    }
};

struct expression
{
};

struct boolean_expression
{
};

struct query_object
{
    virtual void run() { }
};

struct exit_t : query_object
{
    void run() override
    {
        std::cerr << "Bye bye" << std::endl;
        exit(0);
    }
};

struct describe : query_object
{
    std::vector<table*> tables;

    describe(parse_tree_node node,
             std::unordered_map<std::string, table>& tables_)
    {
        if(!node.args.size())
        {
            std::cerr << "INTERNAL: No args to DESCRIBE." << std::endl;
            throw 0;
        }

        for(auto& arg : node.args)
        {
            identitifer table_id = identitifer(arg);
            auto table_name = table_id.id;

            if(tables_.find(table_name) == tables_.end())
            {
                std::cerr << "Could not resolve table name " << table_name << "." << std::endl;
                throw 0;
            }

            tables.push_back(&tables_[table_name]);
        }
    }

    void run() override
    {
        for(auto& t : tables) t->describe();
    }
};

struct show : query_object
{
    std::unordered_map<std::string, table>* tables;

    show(parse_tree_node& node,
         std::unordered_map<std::string, table>& tables_)
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

struct load : query_object
{
    std::unordered_map<std::string, table> tables;
    std::vector<as<identitifer>> load_args;

    load(parse_tree_node& node,
         std::unordered_map<std::string, table>& tables)
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

            load_args.push_back(as<identitifer>(arg));
        }
    }

    void run() override
    {
        for(auto& load_arg : load_args)
        {
            auto csv = load_arg.value.id;
            auto table_name = load_arg.name;

            if(tables.find(table_name) != tables.end())
            {
                std::cerr << "Invalid input: Attempted to load more than one"
                          << "table of the same name.    " << table_name << std::endl;
                throw 0;
            }

            tables.emplace(std::make_pair(table_name, table(csv)));
        }
    }
};
/*
struct select : query_object, table_view
{
    std::vector<expression> columns;
    table_view* from;
    std::vector<boolean_expression> where;
    long long int limit;
    long long int offset;

    select(parse_tree_node& node,
           std::unordered_map<std::string, table>& tables)
    {
        if(node.args.size() < 2)
        {
            std::cerr << "
        }
    }
};*/

std::unique_ptr<query_object> compile_query(parse_tree_node& node,
                                            std::unordered_map<std::string, table>& tables)
{
    switch(node.token.t)
    {
        case token_t::DESCRIBE:
        {
            return std::unique_ptr<query_object>(new describe(node, tables));
        }
        case token_t::EXIT:
        {
            return std::unique_ptr<query_object>(new exit_t());
        }
        case token_t::SHOW:
        {
            return std::unique_ptr<query_object>(new show(node, tables));
        }
        case token_t::LOAD:
        {
            return std::unique_ptr<query_object>(new load(node, tables));
        }
        case token_t::SELECT:
        {
            return std::unique_ptr<query_object>(new load(node, tables));
        }
    }
};

#endif

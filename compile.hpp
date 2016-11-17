#ifndef _COMPILE_H
#define _COMPILE_H

#include <iostream>
#include <memory>
#include <unordered_map>

#include "lexer.hpp"
#include "parser.hpp"

struct query_object
{
    virtual void run();
}

struct exit : query_object
{
    override void run()
    {
        cerr << "Bye bye" << std::endl;
        exit(0);
    }
}

struct describe : query_object
{
    table* t;

    describe(parse_tree_node* parse_tree,
             std::unordered_map<std::string, table> tables;
    {

    }
}

std::unique_ptr<query_object> compile_query(parse_tree_node parse_tree)
{
    switch(parse_tree_node.token.t)
    {
        case DESCRIBE:
        {
            return unique_ptr<query_object>(new describe(parse_tree));
        }
    }
}

#endif

#ifndef _PARSER_H
#define _PARSER_H

#include <iostream>
#include <vector>

#include "lexer.hpp"
#include "util.hpp"

struct parse_tree_node
{
    // Keep track of whether or not we're a value or unbound operation
    enum abstract_type
    {
        // Intermediatary states
        EMPTY,
        OPERATION,

        VALUE,
    };

    abstract_type a_type;
    token_t token;
    std::vector<parse_tree_node> args;

    parse_tree_node() :
        a_type(EMPTY), token()       , args() {};
    parse_tree_node(abstract_type a, token_t token_) :
        a_type(a),          token(token_) , args() {};
    parse_tree_node(token_t token_, std::vector<parse_tree_node> args_) :
        a_type(VALUE),      token(token_) , args(args_) {};

    void print()
    {
        print(0);
    }
    void print(int level)
    {
        for(int i = 0; i < level; i++) std::cerr << "    ";
        std::cerr << output_token(token) << std::endl;
        for(auto& arg: args) arg.print(level+1);
    }
};

parse_tree_node parse(std::vector<token_t>& tokens);

#endif

#ifndef _RPN_PARSER_H
#define _RPN_PARSER_H

#include <iostream>
#include <queue>
#include <stack>

#include "lexer.hpp"
#include "util.hpp"

#define DEBUG

int precedence(token_t t)
{
    switch(t.t)
    {
        case token_t::PAREN_OPEN:
            return 0;
        case token_t::SELECT:    case token_t::SHOW:  case token_t::DESCRIBE:
        case token_t::LOAD:
            return 1;
        case token_t::LIMIT:     case token_t::OFFSET:
            return 2;
        case token_t::WHERE:
            return 3;
        case token_t::FROM:
            return 4;
        case token_t::JOIN:      case token_t::AS:
            return 5;
        case token_t::ON:
            return 6;
        case token_t::PLUS:      case token_t::MINUS:
            return 7;
        case token_t::STAR:      case token_t::DIVIDE:    case token_t::MOD:
            return 8;
        case token_t::CARAT:
            return 9;
        case token_t::EQUAL:     case token_t::NEQUAL:    case token_t::LT:
        case token_t::LTEQ:      case token_t::GT:        case token_t::GTEQ:
            return 10;
        case token_t::AND:
            return 11;
        case token_t::OR:
            return 12;
        case token_t::NEGATE:   case token_t::BANG:
            return 13;
    }

    std::cerr << "INTERNAL: Attempted to resolve precedence for token of type "
              << output_token(t) << "." << std::endl;
    throw 0;
}

struct parse_tree_node
{
    enum abstract_type
    {
        // Intermediatory states
        UNRESOLVED,

        // Concrete states
        VALUE,
        BOOL,
        INT,
        FLOAT,
        STR,
        IDENTIFIER,
        OPERATION,
        ARITH_RES,
        LOGICAL_RES,
        TABLE_VIEW,
        ROW_VIEW
    };

    abstract_type a_type;
    token_t token;
    std::vector<parse_tree_node> args;

    parse_tree_node() :
        a_type(UNRESOLVED), token()       , args() {};
    parse_tree_node(token_t token_) :
        a_type(VALUE),      token(token_) , args() {};
    parse_tree_node(token_t token_, std::vector<parse_tree_node> args_) :
        a_type(VALUE),      token(token_) , args(args_) {};
    parse_tree_node(abstract_type t_, token_t token_) :
        a_type(t_),         token(token_) , args() {};
    parse_tree_node(abstract_type t_) :
        a_type(t_),         token()       , args() {};
};

void print_parse_tree(parse_tree_node node, int level)
{
    for(int i = 0; i < level; i++) std::cout << "    ";
    std::cout << output_token(node.token) << std::endl;
    for(auto& t: node.args) print_parse_tree(t, level+1);
}

parse_tree_node bind_binary(token_t operation,
                            std::vector<parse_tree_node>& parse_tree)
{
    if(parse_tree.size() < 3)
    {
        std::cerr << "Not enough arguments to bind operator: "
                  << output_token(operation) << std::endl;
        throw 0;
    }

    parse_tree_node right = pop_back(parse_tree);
    parse_tree_node oper  = pop_back(parse_tree);
    parse_tree_node left  = pop_back(parse_tree);
    if(oper.token.t != operation.t)
    {
        std::cerr << "Error when attempting to bind: "
                  << output_token(operation) << std::endl;
        throw 0;
    }

    return parse_tree_node(operation, std::vector<parse_tree_node>{left, right});
/*
    switch(operation.token.t)
    {
        case token_t::PLUS:  case token_t::MINUS:
        case token_t::STAR:  case token_t::DIVIDE:
        case token_t::CARAT:
        {
            if(right.a_type != parse_tree_node::VALUE)
            {
                cerr << "Operator " << output_token(operation)
                     << "requires a value type right argument.";
            }

            if(left.atype   != parse_tree_node::VALUE)
            {
                cerr << "Operator " << output_token(operation)
                     << "requires a value type left argument.";
            }

            if(left.c_type == parse_tree_node::INT
            {
            }
        }
        case token_t::MOD:
        {
            break;
        }
        case token_t::LT:    case token_t::LTEQ:
        case token_t::GT:    case token_t::GTEQ:
        case token_t::AND:   case token_t::OR:
        {
            break;
        }
        case token_t::AS:
        {

        }
    }*/
}

void bind_top(std::vector<token_t>& operations,
          std::vector<parse_tree_node>& parse_tree)
{
    auto op = pop_back(operations);

    std::cerr << output_token(op) << " " << op.t << " " << token_t::FROM << std::endl;
    switch(op.t)
    {
        // BINARY infix
        case token_t::PLUS:  case token_t::MINUS:
        case token_t::STAR:  case token_t::DIVIDE:
        case token_t::MOD:   case token_t::CARAT:
        case token_t::EQUAL: case token_t::NEQUAL:
        case token_t::LT:    case token_t::LTEQ:
        case token_t::GT:    case token_t::GTEQ:
        case token_t::AND:   case token_t::OR:
        case token_t::AS:
        {
            parse_tree.push_back(bind_binary(op, parse_tree));
            return;
        }
        // Variadic
        case token_t::SELECT: case token_t::FROM:
        case token_t::WHERE:  case token_t::LIMIT:
        case token_t::LOAD:
        {
            std::vector<parse_tree_node> arg_list;
            while(parse_tree.size() &&
                  parse_tree.back().token.t != op.t)
            {
                if(parse_tree.back().token.t != token_t::COMMA)
                {
                    arg_list.push_back(pop_back(parse_tree));
                }
                else
                {
                    parse_tree.pop_back();
                }
            }
            parse_tree.pop_back();
            parse_tree.push_back(parse_tree_node(op, arg_list));
            return;
        }
        // Join is variabic to the right (ON clause) but takes 1 left argument
        case token_t::JOIN:
        {
            std::vector<parse_tree_node> arg_list;
            while(parse_tree.size() &&
                  parse_tree.back().token.t != op.t)
            {
                arg_list.push_back(pop_back(parse_tree));
            }
            parse_tree.pop_back();
            if(arg_list.size() > 2)
            {
                std::cerr << "Attempting to bind too many arguments to JOIN" << std::endl;
            }
            arg_list.push_back(pop_back(parse_tree));
            parse_tree.push_back(parse_tree_node(op, arg_list));
            return;
        }
        // Unary
        case token_t::OFFSET:   case token_t::SHOW:
        case token_t::DESCRIBE: case token_t::ON:
        case token_t::BANG:     case token_t::NEGATE:
        {
            std::vector<parse_tree_node> arg_list;
            arg_list.push_back(pop_back(parse_tree));
            parse_tree.pop_back();
            parse_tree.push_back(parse_tree_node(op, arg_list));
            return;
        }
    }

    std::cerr << "INTERNAL: Attempted to bind invalid operation "
              << output_token(op) << "." << std::endl;
    throw 0;
}

// Really big method, but I personally find the code more understandable
// with all the logic inline here.

// Processing all the tokens
parse_tree_node to_parse_tree(std::vector<token_t>& tokens)
{
    std::vector<parse_tree_node> parse_tree;
    std::vector<token_t> operations;

    for(auto& token : tokens)
    {
#ifdef DEBUG
        std::cout << std::endl << "--------------------" << std::endl;
        std::cout << std::endl << "--------------------" << std::endl;
        for(auto & o : operations) std::cout << output_token(o) << " ";
        std::cout << std::endl;
        for(auto & p : parse_tree) std::cout << output_token(p.token) << " ";
        std::cout << std::endl << output_token(token) << std::endl;
#endif

        switch(token.t)
        {
            case token_t::FLOAT_LITERAL: case token_t::INT_LITERAL:
            case token_t::STR_LITERAL:   case token_t::IDENTITIFER:
            case token_t::TABLES:        case token_t::EXIT:
            {
                parse_tree.push_back(parse_tree_node(parse_tree_node::VALUE, token));
                break;
            }
            case token_t::PAREN_OPEN:    case token_t::FUNCTION:
            {
                operations.push_back(token);
                parse_tree.push_back(parse_tree_node(token));
                break;
            }
            case token_t::PAREN_CLOSE:
            {
                while(operations.size() && operations.back().t != token_t::PAREN_OPEN)
                {
                    bind_top(operations, parse_tree);
                }
                operations.pop_back(); // PAREN_OPEN

                std::stack<parse_tree_node> arg_list;
                while(parse_tree.size() && parse_tree.back().token.t != token_t::PAREN_OPEN)
                {
                    arg_list.push(pop_back(parse_tree));
                }
                parse_tree.pop_back(); // PAREN_OPEN

                if(parse_tree.size() && parse_tree.back().token.t == token_t::FUNCTION)
                {
                    while(!arg_list.empty()) parse_tree.back().args.push_back(pop_top(arg_list));
                    operations.pop_back(); //FUNCTION
                }
                else // Tuples not supported
                {
                    if(arg_list.size() > 1)
                    {
                        std::cerr << "Tried to push multiple args to non-function." << std::endl;
                        throw 0;
                    }
                    else if(arg_list.size() == 1)
                    {
                        parse_tree.push_back(pop_top(arg_list));
                    }
                }
                break;
            }
            case token_t::COMMA:
            {
                // 3 Magic number, all operatoins < than 3 are variadic,
                // All operation >= are not.
                // Comma bind_toping everything left until the first variadic operations.
                while(operations.size() && precedence(operations.back().t) > 3)
                {
                    bind_top(operations, parse_tree);
                }
                // Pushing comma prevents evaluation of expressions that
                // would otherwise be valid across comma boundary. EG
                // f(a + b, * c) being evaulated as f(a+b*c).
                // Must be ignored when bind_toping variadic operations.
                parse_tree.push_back(token);
                break;
            }
            // Resolve the semantics of minus and star symbol in the here.
            case token_t::MINUS:
            {
                if(parse_tree.empty() ||
                   parse_tree.back().a_type != parse_tree_node::VALUE)
                {
                    token.t = token_t::NEGATE;
                    operations.push_back(token);
                    parse_tree.push_back(parse_tree_node(token));
                }
                goto DEFAULT;
            }
            case token_t::STAR:
            {
                if(parse_tree.empty())
                {
                    std::cerr << "Could not resolve *." << std::endl;
                    throw 0;
                }

                if(parse_tree.back().a_type != parse_tree_node::VALUE)
                {
                    token.t = token_t::SELECT_ALL;
                    parse_tree.push_back(parse_tree_node(token));
                }
            }
            DEFAULT:
            default:
            {
                while(operations.size() &&
                      precedence(operations.back()) >= precedence(token))
                {
                    bind_top(operations, parse_tree);
                }
                operations.push_back(token);
                parse_tree.push_back(parse_tree_node(token));
                break;
            }
        }
    }

    while(operations.size())
    {
#ifdef DEBUG
        std::cerr << "--------------------" << std::endl;
        for(auto & o : operations) std::cerr << output_token(o) << " ";
        std::cerr << std::endl;
        for(auto & p : parse_tree) std::cerr << output_token(p.token) << " ";
#endif
        bind_top(operations, parse_tree);
    }

#ifdef DEBUG
    for(auto& tree: parse_tree)
    {
        std::cerr << std::endl << std::endl;
        print_parse_tree(parse_tree.back(), 0);
    }
#else
    if(parse_tree.size() > 1)
    {
        for(auto& tree: parse_tree)
        {
            std::cerr << std::endl << std::endl;
            print_parse_tree(parse_tree.back(), 0);
        }
    }
#endif

    if(parse_tree.size() > 1)
    {
        std::cerr << "Unbound expressions after attempting to build parse tree." << std::endl;
        throw 0;
    }

    return pop_back(parse_tree);
}

void parse(std::vector<token_t>& tokens)
{
    auto t = to_parse_tree(tokens);
}

#endif

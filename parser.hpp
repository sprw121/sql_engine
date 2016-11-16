#ifndef _RPN_PARSER_H
#define _RPN_PARSER_H

#include <iostream>
#include <queue>
#include <stack>

#include "lexer.hpp"
#include "util.hpp"

/*
enum associativity
{
    LEFT,
    RIGHT
};*/

int precedence(token_t t)
{
    switch(t.t)
    {
        case token_type::PAREN_OPEN:
            return 0;
        case token_type::SELECT:    case token_type::SHOW:  case token_type::DESCRIBE:
        case token_type::LOAD:
            return 1;
        case token_type::LIMIT:     case token_type::OFFSET:
            return 2;
        case token_type::WHERE:
            return 3;
        case token_type::FROM:
            return 4;
        case token_type::JOIN:      case token_type::AS:
            return 5;
        case token_type::ON:
            return 6;
        case token_type::PLUS:      case token_type::MINUS:
            return 7;
        case token_type::STAR:      case token_type::DIVIDE:    case token_type::MOD:
            return 8;
        case token_type::CARAT:
            return 9;
        case token_type::EQUAL:     case token_type::NEQUAL:    case token_type::LT:
        case token_type::LTEQ:      case token_type::GT:        case token_type::GTEQ:
            return 10;
        case token_type::AND:
            return 11;
        case token_type::OR:
            return 12;
    }

    std::cerr << "INTERNAL: Attempted to resolve precedence for token of type "
              << output_token(t) << "." << std::endl;
    throw 0;
}

struct parse_tree_node
{
    enum node_type
    {
        // Intermediatory states
        EMPTY,
        TOKEN,

        // Concrete states
        VALUE,
        EXP_RES,
        JOIN,
    };

    node_type t;
    token_t token;
    std::vector<parse_tree_node> args;

    parse_tree_node() :
        t(EMPTY), token() , args(std::vector<parse_tree_node>()) {};
    parse_tree_node(token_t token_) :
        t(VALUE), token(token_) , args(std::vector<parse_tree_node>()) {};
    parse_tree_node(node_type t_, token_t token_) :
        t(t_), token(token_) , args(std::vector<parse_tree_node>()) {};
    parse_tree_node(node_type t_) :
        t(t_), token() , args() {};
};

void print_parse_tree(parse_tree_node node, int level)
{
    for(int i = 0; i < level; i++) std::cout << "    ";
    std::cout << output_token(node.token) << std::endl;
    for(auto& t: node.args) print_parse_tree(t, level+1);
}

void bind(std::vector<parse_tree_node>& operators,
          std::vector<parse_tree_node>& parse_tree)
{
    auto op = pop_back(operators);

    switch(op.token.t)
    {
        // BINARY infix
        case token_type::PLUS:  case token_type::MINUS:
        case token_type::STAR:  case token_type::DIVIDE:
        case token_type::MOD:   case token_type::CARAT:
        case token_type::EQUAL: case token_type::NEQUAL:
        case token_type::LT:    case token_type::LTEQ:
        case token_type::GT:    case token_type::GTEQ:
        case token_type::AND:   case token_type::OR:
        case token_type::AS:
        {
            parse_tree_node right = pop_back(parse_tree);
            parse_tree_node oper = pop_back(parse_tree);
            parse_tree_node left = pop_back(parse_tree);
            if(oper.token.t != op.token.t)
            {
                std::cerr << "Invalid expression.";
                throw 0;
            }
            op.args.push_back(left);
            op.args.push_back(right);
            op.t = parse_tree_node::EXP_RES;
            parse_tree.push_back(op);
            break;
        }
        // Variadic
        case token_type::SELECT: case token_type::FROM:
        case token_type::WHERE:  case token_type::LIMIT:
        case token_type::LOAD:
        {
            while(parse_tree.size() &&
                  parse_tree.back().token.t != op.token.t)
            {
                if(parse_tree.back().token.t != token_type::COMMA)
                {
                    op.args.push_back(pop_back(parse_tree));
                }
                else
                {
                    parse_tree.pop_back();
                }
            }
            parse_tree.pop_back();
            parse_tree.push_back(op);
            break;
        }
        // Join is variabic to the right (ON clause) but takes 1 left argument
        case token_type::JOIN:
        {
            while(parse_tree.size() &&
                  parse_tree.back().token.t != op.token.t)
            {
                op.args.push_back(pop_back(parse_tree));
            }
            parse_tree.pop_back();
            if(op.args.size() > 2)
            {
                std::cerr << "Invalid expression";
            }
            op.args.push_back(pop_back(parse_tree));
            parse_tree.push_back(op);
            break;
        }
        // Unary
        case token_type::OFFSET:   case token_type::SHOW:
        case token_type::DESCRIBE: case token_type::ON:
        {
            op.args.push_back(pop_back(parse_tree));
            parse_tree.pop_back();
            parse_tree.push_back(op);
            break;
        }
        default:
        {
            std::cerr << "INTERNAL: Attempted to resolve precedence for invalid operator "
                      << output_token(op.token) << ".";
            throw 0;
        }
    }
}

parse_tree_node to_parse_tree(std::vector<token_t>& tokens)
{
    std::vector<parse_tree_node> parse_tree;
    std::vector<parse_tree_node> operators;

    for(auto& token : tokens)
    {
        std::cout << std::endl << "--------------------" << std::endl;
        std::cout << std::endl << "--------------------" << std::endl;
        for(auto & o : operators) std::cout << output_token(o.token) << " ";
        std::cout << std::endl;
        for(auto & p : parse_tree) std::cout << output_token(p.token) << " ";
        std::cout << std::endl << output_token(token) << std::endl;

        switch(token.t)
        {
            case token_type::FLOAT_LITERAL: case token_type::INT_LITERAL:
            case token_type::STR_LITERAL:   case token_type::IDENTITIFER:
            case token_type::TABLES:        case token_type::EXIT:
            {
                parse_tree.push_back(parse_tree_node(parse_tree_node::VALUE, token));
                break;
            }
            case token_type::PAREN_OPEN:    case token_type::FUNCTION:
            {
                operators.push_back(parse_tree_node(token));
                parse_tree.push_back(parse_tree_node(token));
                break;
            }
            case token_type::PAREN_CLOSE:
            {
                while(operators.size() && operators.back().token.t != token_type::PAREN_OPEN)
                {
                    bind(operators, parse_tree);
                }
                operators.pop_back(); // PAREN_OPEN

                std::stack<parse_tree_node> arg_list;
                while(parse_tree.size() && parse_tree.back().token.t != token_type::PAREN_OPEN)
                {
                    arg_list.push(pop_back(parse_tree));
                }
                parse_tree.pop_back(); // PAREN_OPEN

                if(parse_tree.size() && parse_tree.back().token.t == token_type::FUNCTION)
                {
                    while(!arg_list.empty()) parse_tree.back().args.push_back(pop_top(arg_list));
                    operators.pop_back(); //FUNCTION
                }
                else // Tuples not supported
                {
                    if(arg_list.size() > 1)
                    {
                        std::cerr << "Tried to push multiple args to non-function.";
                        throw 0;
                    }
                    else if(arg_list.size() == 1)
                    {
                        parse_tree.push_back(pop_top(arg_list));
                    }
                }
                break;
            }
            case token_type::COMMA:
            {
                // 3 Magic number, all operatoins < than 3 are variadic,
                // All operation >= are not.
                // Comma binding everything left until the first variadic operations.
                while(operators.size() && precedence(operators.back().token.t) > 3)
                {
                    bind(operators, parse_tree);
                }
                // Pushing comma prevents evaluation of expressions that
                // would otherwise be valid across comma boundary. EG
                // f(a + b, * c) being evaulated as f(a+b*c).
                // Must be ignored when binding variadic operations.
                parse_tree.push_back(token);
                break;
            }
            default:
            {
                while(operators.size() &&
                      precedence(operators.back().token) >= precedence(token))
                {
                    bind(operators, parse_tree);
                }
                operators.push_back(parse_tree_node(token));
                parse_tree.push_back(parse_tree_node(token));
                break;
            }
        }
    }

    while(operators.size())
    {
        std::cout << std::endl << "--------------------" << std::endl;
        for(auto & o : operators) std::cout << output_token(o.token) << " ";
        std::cout << std::endl;
        for(auto & p : parse_tree) std::cout << output_token(p.token) << " ";
        bind(operators, parse_tree);
    }

    std::cout << std::endl << std::endl;

    print_parse_tree(parse_tree.back(), 0);

    return pop_back(parse_tree);
}

void parse(std::vector<token_t>& tokens)
{
    auto t = to_parse_tree(tokens);
}
/*
std::vector<token_t> to_rpn(std::vector<token_t>& tokens)
{
    std::vector<token_t> output_buffer;
    std::stack<token_t> operators;

    for(auto& token: tokens)
    {
        switch(token.t)
        {
            case token_type::FLOAT_LITERAL: case token_type::INT_LITERAL:
            case token_type::STR_LITERAL:   case token_type::IDENTITIFER:
            case token_type::TABLES:
                output_buffer.push_back(token);
                break;
            case token_type::FUNCTION:      case token_type::PAREN_OPEN:
                operators.push(token);
                break;
            case token_type::PAREN_CLOSE:
                while(operators.size() && operators.top().t != token_type::PAREN_OPEN)
                {
                    output_buffer.push_back(operators.top());
                    operators.pop();
                }

                if(!operators.size())
                {
                    std::cerr << "Unmatched left parenthesis";
                    throw 0;
                }

                operators.pop();
                if(operators.size() && operators.top().t == token_type::FUNCTION)
                {
                    output_buffer.push_back(operators.top());
                    operators.pop();
                }
                break;
            case token_type::COMMA:
                while(operators.size() &&
                      (resolve_precedence(operators.top()) > 0))
                {
                    output_buffer.push_back(operators.top());
                    operators.pop();
                }

                if(!operators.size())
                {
                    std::cerr << "Comma unmatched with command or parenthesis.";
                    throw 0;
                }
                break;
            default:
                while(operators.size() &&
                      resolve_precedence(operators.top()) >= resolve_precedence(token))
                {
                    output_buffer.push_back(operators.top());
                    operators.pop();
                }

                operators.push(token);

                break;
        }
    }

    while(operators.size())
    {
        output_buffer.push_back(operators.top());
        operators.pop();
    }

    for(auto e : output_buffer)
    {
        std::cout << output_token(e) << std::endl;
    }

    return output_buffer;
}

struct ast_node
{
    enum ast_node_type
    {
        VALUE,
        ARITH_RES,
        LOGICAL_RES,
        TABLE_VIEW,
        FROM,
        WHERE,
        LIMIT,
        OFFSET,
        SHOW,
        DESCRIBE,
        EXIT,
        LOAD,
        SELECT,
        INVALID
    };

    ast_node() : t(ast_node_type::INVALID) {};
    ast_node(ast_node_type t_, token_t origin_) : t(t_), origin(origin_) {};
    ast_node(ast_node_type t_,
             token_t origin_,
             std::vector<ast_node> args_) : t(t_), origin(origin_), args(args_) {};

    ast_node_type t;
    token_t origin;
    std::vector<ast_node> args;
};

std::string output_ast_type(ast_node::ast_node_type t)
{
    switch(t)
    {
        case ast_node::VALUE:       return "VALUE";
        case ast_node::ARITH_RES:   return "ARITH_RES";
        case ast_node::LOGICAL_RES: return "LOGICAL_RES";
        case ast_node::TABLE_VIEW:  return "TABLE_VIEW";
        case ast_node::FROM:        return "FROM";
        case ast_node::WHERE:       return "WHERE";
        case ast_node::LIMIT:       return "LIMIT";
        case ast_node::OFFSET:      return "OFFSET";
        case ast_node::SHOW:        return "SHOW";
        case ast_node::DESCRIBE:    return "DESCRIBE";
        case ast_node::EXIT:        return "EXIT";
        case ast_node::LOAD:        return "LOAD";
        case ast_node::SELECT:      return "SELECT";
        case ast_node::INVALID:     return "INVALID";
    }
}

void print_ast(ast_node node, int level)
{
    for(int i = 0; i < level; i++) std::cout << "    ";
    std::cout << output_ast_type(node.t) << " (" << output_token(node.origin) << ")" << std::endl;
    for(auto& n: node.args) print_ast(n, level + 1);
}*/

/*
void parse(std::vector<token_t>& tokens)
{
    //auto rpn_tokens = to_rpn(tokens);

    std::stack<ast_node> ast;
    for(auto& token : rpn_tokens)
    {
        switch(token.t)
        {
            case token_type::FLOAT_LITERAL: case token_type::INT_LITERAL:
            case token_type::STR_LITERAL:   case token_type::IDENTITIFER:
            case token_type::TABLES:
                ast.push(ast_node(ast_node::VALUE, token));
                break;
            case token_type::PLUS: case token_type::MINUS:
            case token_type::STAR: case token_type::DIVIDE:
            case token_type::MOD:  case token_type::CARAT:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));
                args.push_back(pop_top(ast));
                std::cout << args[0].t << " " << args[1].t << std::endl;

                if((args[0].t == ast_node::VALUE || args[0].t == ast_node::ARITH_RES) &&
                    args[1].t == ast_node::VALUE || args[1].t == ast_node::ARITH_RES)
                {
                    ast.push(ast_node(ast_node::ARITH_RES, token, args));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::EQUAL: case token_type::NEQUAL:
            case token_type::LT:    case token_type::LTEQ:
            case token_type::GT:    case token_type::GTEQ:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));
                args.push_back(pop_top(ast));
                std::cout << args[0].t << " " << args[1].t << std::endl;

                if((args[0].t == ast_node::VALUE || args[0].t == ast_node::ARITH_RES) &&
                    args[1].t == ast_node::VALUE || args[1].t == ast_node::ARITH_RES)
                {
                    ast.push(ast_node(ast_node::LOGICAL_RES, token, args));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::AND:   case token_type::OR:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));
                args.push_back(pop_top(ast));
                std::cout << args[0].t << " " << args[1].t << std::endl;

                if((args[0].t == ast_node::LOGICAL_RES) &&
                    args[1].t == ast_node::LOGICAL_RES)
                {
                    ast.push(ast_node(ast_node::LOGICAL_RES, token, args));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::FROM:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));
                std::cout << args[0].t << std::endl;

                if(args[0].t == ast_node::TABLE_VIEW ||
                   args[0].t == ast_node::VALUE)
                {
                    ast.push(ast_node(ast_node::FROM, token, args));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::WHERE:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));
                std::cout << args[0].t << std::endl;

                if(args[0].t == ast_node::LOGICAL_RES)
                {
                    ast.push(ast_node(ast_node::WHERE, token, args));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }

            case token_type::LIMIT:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));
                std::cout << args[0].t << std::endl;

                if(args[0].t == ast_node::VALUE || args[0].t == ast_node::ARITH_RES)
                {
                    ast.push(ast_node(ast_node::LIMIT, token, args));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::OFFSET:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));
                std::cout << args[0].t << std::endl;

                if(args[0].t == ast_node::VALUE || args[0].t == ast_node::ARITH_RES)
                {
                    ast.push(ast_node(ast_node::OFFSET, token, args));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::JOIN:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));
                args.push_back(pop_top(ast));

                std::cout << args[0].t << " " << args[1].t << std::endl;

                if((args[0].t == ast_node::VALUE || args[0].t == ast_node::TABLE_VIEW) &&
                    args[1].t == ast_node::VALUE || args[1].t == ast_node::TABLE_VIEW)
                {
                    ast.push(ast_node(ast_node::TABLE_VIEW, token, args));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::ON:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));
                args.push_back(pop_top(ast));

                std::cout << args[0].t << " " << args[1].t << std::endl;

                if((args[0].t == ast_node::VALUE || args[0].t == ast_node::TABLE_VIEW) &&
                    args[1].t == ast_node::VALUE || args[1].t == ast_node::TABLE_VIEW)
                {
                    ast.push(ast_node(ast_node::TABLE_VIEW, token, args));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::AS:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));
                args.push_back(pop_top(ast));

                std::cout << args[0].t << " " << args[1].t << std::endl;

                if(args[0].t == ast_node::VALUE &&
                        (args[1].t == ast_node::VALUE ||
                         args[1].t == ast_node::TABLE_VIEW ||
                         args[1].t == ast_node::ARITH_RES))
                {
                    ast.push(ast_node(args[1].t, token, args));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::SHOW:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));

                std::cout << args[0].t << " " << std::endl;

                if(args[0].t != ast_node::VALUE)
                {
                    ast.push(ast_node(ast_node::SHOW, token));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::DESCRIBE:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));

                std::cout << args[0].t << " " << std::endl;

                if(args[0].t != ast_node::VALUE)
                {
                    ast.push(ast_node(ast_node::DESCRIBE, token));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::EXIT:
            {
                if(ast.size() != 0)
                {
                    std::cerr << "Exit expects no arguments.";
                    throw 0;
                }

                ast.push(ast_node(ast_node::EXIT, token));
                break;
            }
            case token_type::LOAD:
            {
                std::vector<ast_node> args;
                args.push_back(pop_top(ast));

                std::cout << args[0].t << " " << std::endl;

                if(args[0].t != ast_node::ast_node::VALUE)
                {
                    ast.push(ast_node(ast_node::LOAD, token));
                }
                else
                {
                    std::cerr << "Invalid expressions types for operator "
                              << output_token(token) << ".";
                    throw 0;
                }
                break;
            }
            case token_type::SELECT:
            {
                std::vector<ast_node> args;

                while(ast.size())
                {
                    args.push_back(pop_top(ast));
                }

                ast.push(ast_node(ast_node::SELECT, token, args));
                break;
            }
        }
    }

    if(ast.size() != 1)
    {
        std::cerr << "Invalid AST." << std::endl;
        throw 0;
    }

    print_ast(ast.top(), 0);

    return pop_top(ast);
}*/

#endif

#include "lexer.hpp"
#include "parser.hpp"

//#define DEBUG

int precedence(token_t t)
{
    switch(t.t)
    {
        case token_t::PAREN_OPEN:
            return 0;
        case token_t::SELECT:     case token_t::SHOW:  case token_t::DESCRIBE:
        case token_t::LOAD:
            return 1;
        case token_t::LIMIT:      case token_t::OFFSET:
            return 2;
        case token_t::WHERE:
            return 3;
        case token_t::FROM:
            return 4;
        case token_t::AS:         case token_t::LEFT_JOIN:  case token_t::CROSS_JOIN:
        case token_t::RIGHT_JOIN: case token_t::OUTER_JOIN: case token_t::INNER_JOIN:
            return 5;
        case token_t::ON:
            return 6;
        case token_t::OR:
            return 7;
        case token_t::AND:
            return 8;
        case token_t::EQUAL:      case token_t::NEQUAL:    case token_t::LT:
        case token_t::LTEQ:       case token_t::GT:        case token_t::GTEQ:
            return 9;
        case token_t::PLUS:       case token_t::MINUS:
            return 10;
        case token_t::STAR:       case token_t::DIVIDE:    case token_t::MOD:
            return 11;
        case token_t::CARAT:
            return 12;
        case token_t::NEGATE:    case token_t::BANG:
            return 13;
    }

    std::cerr << "INTERNAL: Attempted to resolve precedence for token of type "
              << output_token(t) << "." << std::endl;
    throw 0;
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
}

void bind_top(std::vector<token_t>& operations,
          std::vector<parse_tree_node>& parse_tree)
{
    auto op = pop_back(operations);

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
        case token_t::AS:    case token_t::CROSS_JOIN:
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
        // Joins take 2 right args and 1 left argument
        case token_t::LEFT_JOIN:    case token_t::RIGHT_JOIN:
        case token_t::OUTER_JOIN:   case token_t::INNER_JOIN:
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

// Really big method, some would break out the logic for each case
// into a function, but I personally find the code more understandable
// with all the logic inline here.

// Processes all the tokens
parse_tree_node parse(std::vector<token_t>& tokens)
{
    std::vector<parse_tree_node> parse_tree;
    std::vector<token_t> operations;

    for(auto& token : tokens)
    {
#ifdef DEBUG
        std::cerr << std::endl << "--------------------" << std::endl;
        std::cerr << std::endl << "--------------------" << std::endl;
        for(auto & o : operations) std::cerr << output_token(o) << " ";
        std::cerr << std::endl;
        for(auto & p : parse_tree) std::cerr << output_token(p.token) << " ";
        std::cerr << std::endl << output_token(token) << std::endl;
#endif

        switch(token.t)
        {
            case token_t::FLOAT_LITERAL: case token_t::INT_LITERAL:
            case token_t::STR_LITERAL:   case token_t::IDENTITIFER:
            case token_t::TABLES:        case token_t::EXIT:
            {
                if(parse_tree.empty() || parse_tree.back().a_type == parse_tree_node::OPERATION)
                {
                    parse_tree.push_back(parse_tree_node(parse_tree_node::VALUE, token));
                }
                else
                {
                    std::cerr << "Attempting to push unbindable value type: "
                              << output_token(token) << std::endl;
                    throw 0;
                }
                break;
            }
            case token_t::PAREN_OPEN:    case token_t::FUNCTION:
            {
                operations.push_back(token);
                parse_tree.push_back(parse_tree_node(parse_tree_node::OPERATION, token));
                break;
            }
            case token_t::PAREN_CLOSE:
            {
                while(operations.size() && operations.back().t != token_t::PAREN_OPEN)
                {
                    bind_top(operations, parse_tree);
                }
                operations.pop_back(); // PAREN_OPEN

                std::vector<parse_tree_node> arg_list;
                while(parse_tree.size() && parse_tree.back().token.t != token_t::PAREN_OPEN)
                {
                    if(parse_tree.back().token.t != token_t::COMMA)
                    {
                        arg_list.push_back(pop_back(parse_tree));
                    }
                    else
                    {
                        pop_back(parse_tree);
                    }
                }
                parse_tree.pop_back(); // PAREN_OPEN

                if(parse_tree.size() && parse_tree.back().token.t == token_t::FUNCTION)
                {
                    auto tmp = pop_back(operations); //FUNCTION
                    parse_tree.pop_back();           //FUNCTION
                    parse_tree.push_back(parse_tree_node(tmp, arg_list));
                }
                else // Tuples not supported, 1 args == normal expression
                {
                    if(arg_list.size() > 1)
                    {
                        std::cerr << "Tried to push multiple args to non-function." << std::endl;
                        throw 0;
                    }
                    else if(arg_list.size() == 1)
                    {
                        parse_tree.push_back(pop_back(arg_list));
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
                parse_tree.push_back(parse_tree_node(parse_tree_node::OPERATION, token));
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
                    parse_tree.push_back(parse_tree_node(parse_tree_node::OPERATION, token));
                    break; // Break here
                }

                goto DEFAULT; // Else treat normally
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
                    parse_tree.push_back(parse_tree_node(parse_tree_node::OPERATION, token));
                    break; // Break here, otherwise fallthrough
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
                parse_tree.push_back(parse_tree_node(parse_tree_node::OPERATION, token));
                break;
            }
        }
    }

    while(operations.size())
    {
#ifdef DEBUG
        std::cerr << std::endl << "--------------------" << std::endl;
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
        tree.print();
    }
#else
    if(parse_tree.size() > 1)
    {
        for(auto& tree: parse_tree)
        {
            std::cerr << std::endl << std::endl;
            tree.print();
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

#include "../lexer.hpp"
#include "../parser.hpp"

#include "expression.hpp"
#include "expression_impl.hpp"

std::unique_ptr<expression_t> expression_factory(parse_tree_node node, from_t& from)
{
    switch(node.token.t)
    {
        case token_t::IDENTITIFER:
        {
            return std::unique_ptr<expression_t>(new column_accessor(node, from));
        }
        case token_t::INT_LITERAL:
        case token_t::FLOAT_LITERAL:
        case token_t::STR_LITERAL:
        {
            return std::unique_ptr<expression_t>(new const_expr(node,from));
        }
        case token_t::PLUS:
        {
            return std::unique_ptr<expression_t>(new add_t(node,from));
        }
        case token_t::MINUS:
        {
            return std::unique_ptr<expression_t>(new sub_t(node,from));
        }
        case token_t::STAR:
        {
            return std::unique_ptr<expression_t>(new mult_t(node,from));
        }
        case token_t::DIVIDE:
        {
            return std::unique_ptr<expression_t>(new divi_t(node,from));
        }
        case token_t::MOD:
        {
            return std::unique_ptr<expression_t>(new mod_t(node,from));
        }
        case token_t::NEGATE:
        {
            return std::unique_ptr<expression_t>(new negate_t(node,from));
        }
        case token_t::CARAT:
        {
            std::cerr << "Exponentiation not implemented." << std::endl;
            throw 0;
        }
    }

    std::cerr << "Unexpected arg where expected expression. Parse tree:" << std::endl;
    node.print();
    throw 0;
}

std::unique_ptr<expression_t> expression_factory(std::string column, from_t& from)
{
    return std::unique_ptr<expression_t>(new column_accessor(column, from));
}

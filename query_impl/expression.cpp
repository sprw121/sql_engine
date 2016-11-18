#include "../lexer.hpp"
#include "../parser.hpp"

#include "expression.hpp"
#include "expression_impl.hpp"

expression_t::expression_t(parse_tree_node node, from_t& from)
{
    node.print();
    switch(node.token.t)
    {
        case token_t::IDENTITIFER:
        {
            impl = std::unique_ptr<expression_impl>(new column_accessor(node, from));
            break;
        }
        case token_t::INT_LITERAL:
        case token_t::FLOAT_LITERAL:
        case token_t::STR_LITERAL:
        {
            break;
        }
        case token_t::PLUS:
        {
            impl = std::unique_ptr<expression_impl>(new binary_op<add_t>(node,from));
            break;
        }
        case token_t::MINUS:
        {
            impl = std::unique_ptr<expression_impl>(new binary_op<sub_t>(node,from));
            break;
        }
        case token_t::STAR:
        {
            impl = std::unique_ptr<expression_impl>(new binary_op<mult_t>(node,from));
            break;
        }
        case token_t::DIVIDE:
        {
            impl = std::unique_ptr<expression_impl>(new binary_op<divi_t>(node,from));
            break;
        }
        case token_t::MOD:
        {
            impl = std::unique_ptr<expression_impl>(new binary_op<mod_t>(node,from));
            break;
        }
    }
}

cell expression_t::call()
{
    return impl->call();
}

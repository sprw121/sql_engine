#include "../lexer.hpp"
#include "../parser.hpp"

#include <memory>

#include "boolean_expression.hpp"
#include "boolean_expression_impl.hpp"

std::unique_ptr<boolean_expression_t> boolean_factory(parse_tree_node node, from_t& from)
{
    switch(node.token.t)
    {
        case token_t::EQUAL:
        {
            return std::unique_ptr<boolean_expression_t>(new eq_t(node,from));
        }
        case token_t::NEQUAL:
        {
            return std::unique_ptr<boolean_expression_t>(new neq_t(node,from));
        }
        case token_t::LT:
        {
            return std::unique_ptr<boolean_expression_t>(new lt_t(node,from));
        }
        case token_t::LTEQ:
        {
            return std::unique_ptr<boolean_expression_t>(new lteq_t(node,from));
        }
        case token_t::GT:
        {
            return std::unique_ptr<boolean_expression_t>(new gt_t(node,from));
        }
        case token_t::GTEQ:
        {
            return std::unique_ptr<boolean_expression_t>(new gteq_t(node,from));
        }
        case token_t::AND:
        {
            return std::unique_ptr<boolean_expression_t>(new logical_op<and_t>(node,from));
        }
        case token_t::OR:
        {
            return std::unique_ptr<boolean_expression_t>(new logical_op<or_t>(node,from));
        }
        case token_t::BANG:
        {
            return std::unique_ptr<boolean_expression_t>(new not_op(node,from));
        }
    }

    std::cerr << "Unexpected arg where expecting boolean expression. Parse tree: " << std::endl;
    node.print();
    throw 0;
}

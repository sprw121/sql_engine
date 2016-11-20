#include "../lexer.hpp"
#include "../parser.hpp"

#include "boolean_expression.hpp"
#include "boolean_expression_impl.hpp"

boolean_expression_t::boolean_expression_t(parse_tree_node node, from_t& from)
{
    switch(node.token.t)
    {
        case token_t::EQUAL:
        {
            impl = std::unique_ptr<boolean_expression_impl>(new comparison_op<eq_t>(node,from));
            break;
        }
        case token_t::NEQUAL:
        {
            impl = std::unique_ptr<boolean_expression_impl>(new comparison_op<neq_t>(node,from));
            break;
        }
        case token_t::LT:
        {
            impl = std::unique_ptr<boolean_expression_impl>(new comparison_op<lt_t>(node,from));
            break;
        }
        case token_t::LTEQ:
        {
            impl = std::unique_ptr<boolean_expression_impl>(new comparison_op<lteq_t>(node,from));
            break;
        }
        case token_t::GT:
        {
            impl = std::unique_ptr<boolean_expression_impl>(new comparison_op<gt_t>(node,from));
            break;
        }
        case token_t::GTEQ:
        {
            impl = std::unique_ptr<boolean_expression_impl>(new comparison_op<gteq_t>(node,from));
            break;
        }
        case token_t::AND:
        {
            impl = std::unique_ptr<boolean_expression_impl>(new logical_op<and_t>(node,from));
            break;
        }
        case token_t::OR:
        {
            impl = std::unique_ptr<boolean_expression_impl>(new logical_op<or_t>(node,from));
            break;
        }
        case token_t::BANG:
        {
            impl = std::unique_ptr<boolean_expression_impl>(new not_op(node,from));
            break;
        }
    }
}

bool boolean_expression_t::call()
{
    return impl->call();
}

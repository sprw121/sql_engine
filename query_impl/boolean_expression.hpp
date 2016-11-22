#ifndef _BOOLEAN_EXPRESION_H
#define _BOOLEAN_EXPRESION_H

#include "../parser.hpp"

#include "from.hpp"

// Very similar to expression,
// but call returns a bool.

// Should unify with expressions,
// But this was quicker and safer
struct boolean_expression_t
{
    virtual bool call() = 0;
};

std::unique_ptr<boolean_expression_t> boolean_factory(parse_tree_node, from_t&);
#endif

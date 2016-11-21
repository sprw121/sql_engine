#ifndef _COMPILE_H
#define _COMPILE_H

#include <algorithm>
#include <climits>
#include <iostream>
#include <sstream>
#include <memory>
#include <unordered_map>


#include "../lexer.hpp"
#include "../parser.hpp"
#include "../table.hpp"
#include "../table_views.hpp"

#include "as.hpp"
#include "describe.hpp"
#include "exit.hpp"
#include "expression.hpp"
#include "from.hpp"
#include "identitifer.hpp"
#include "limit.hpp"
#include "load.hpp"
#include "offset.hpp"
#include "query_object.hpp"
#include "select.hpp"
#include "show.hpp"
#include "where.hpp"

// This file contains all the code necessary to compile
// a query parse tree into an executable query.

// The general pattern here is for each token to to
// Have it's own struct, and the constructor of that
// struct takes the associated node in the parse_tree
// to construct itself.

// The table_name -> table map from inside the engine
// is also passed around in other to resolve identitifers
// where necessary.

// Commands are implemented as a abstract class type that
// must implement a run method (EXIT, SELECT, DESCRIBE, SHOW, LOAD).

// Certain types (JOIN, SELECT) also implement the table_view
// interface.


std::unique_ptr<query_object> compile_query(parse_tree_node& node,
                                            table_map_t& tables)
{
    switch(node.token.t)
    {
        case token_t::DESCRIBE:
        {
            return std::unique_ptr<query_object>(new describe_t(node, tables));
        }
        case token_t::EXIT:
        {
            return std::unique_ptr<query_object>(new exit_t());
        }
        case token_t::SHOW:
        {
            return std::unique_ptr<query_object>(new show_t(node, tables));
        }
        case token_t::LOAD:
        {
            return std::unique_ptr<query_object>(new load_t(node, tables));
        }
        case token_t::SELECT:
        {
            return select_factory(node, tables);
        }
    }

    std::cerr << "No command seen in query." << std::endl;
    throw 0;
};

#endif

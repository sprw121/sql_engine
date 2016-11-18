#include "from.hpp"

#include "as.hpp"
#include "select.hpp"

from_t::from_t(parse_tree_node node,
               table_map_t& tables)
{
    if(!node.args.size())
    {
        std::cerr << "INTERNAL: No args to FROM." << std::endl;
        throw 0;
    }
    if(node.args.size() != 1)
    {
        std::cerr << "Variadic FROM not supported (1 arg required)" << std::endl;
        throw 0;
    }

    auto& arg = node.args[0];
    switch(arg.token.t)
    {
        case token_t::AS:
        {
            /*
            as_t<select_t> named_select(arg, tables);
            break;
            */
        }
        case token_t::IDENTITIFER:
        {
            view = std::shared_ptr<table_view>(new table_iterator(arg, tables));
            break;
        }
   /*     case token_t::OUTER_JOIN:
        {
            view = std::shared_ptr<table_view>(new outer_join(arg, tables));
        }
        case token_t::INNER_JOIN:
        {
            view = std::shared_ptr<table_view>(new inner_join(arg, tables));
        }
        case token_t::LEFT_JOIN:
        {
            view = std::shared_ptr<table_view>(new left_outer_join(arg, tables));
        }
        case token_t::RIGHT_JOIN:
        {
            view = std::shared_ptr<table_view>(new right_outer_join(arg, tables));
        }
        case token_t::CROSS_JOIN:
        {
            view = std::shared_ptr<table_view>(new cross_join(arg, tables))
            break;
        }*/
        default:
        {
            std::cerr << "Unexpect argument to FROM." << std::endl;
            throw 0;
        }
    }
}

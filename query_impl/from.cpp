#include "from.hpp"

#include "as.hpp"
#include "select.hpp"

// Wraps view construction
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

    auto container = view_factory(node.args[0], tables);
    view = container.view;
}

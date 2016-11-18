#ifndef _SHOW_H
#define _SHOW_H

#include <iostream>

#include "../parser.hpp"
#include "../table.hpp"

#include "query_object.hpp"

struct show_t : query_object
{
    table_map_t* tables;

    show_t() = default;
    show_t(parse_tree_node& node,
         table_map_t& tables_)
    {
        if(!node.args.size())
        {
            std::cerr << "INTERNAL: No args to SHOW." << std::endl;
            throw 0;
        }

        if(node.args.size() != 1 || node.args[0].token.t != token_t::TABLES)
        {
            std::cerr << "Only SHOW TABLES implemented." << std::endl;
            throw 0;
        }

        tables = &tables_;
    }

    void run() override
    {
        std::cout << std::endl;
        if(tables->size() == 0)
        {
            std::cout << "No tables loaded." << std::endl;
        }
        else
        {
            std::cout << "TABLE NAMES" << std::endl
                      << "-----------" << std::endl;
        }
        for(auto& table: *tables)
        {
            std::cout << table.first << std::endl;
        }
        std::cout << std::endl;
    }

};
#endif

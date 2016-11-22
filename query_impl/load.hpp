#ifndef _LOAD_H
#define _LOAD_H

#include <memory>
#include <vector>

#include "../parser.hpp"
#include "../table.hpp"

#include "as.hpp"
#include "identitifer.hpp"

// Load expects a variadic series of
// "as" clauses as arguments,
// i.e. load csv1 as table1, csv2 as table2 ....

// Unpackes the argument list.
// Stores a point back into the table map.
// Run emplaces to emplace the appropriate
// key value pair into the map.
struct load_t : query_object
{
    table_map_t* tables;
    std::vector<as_t<identitifer_t>> load_args;

    load_t() = default;
    load_t(parse_tree_node& node,
           table_map_t& tables_)
    {
        if(!node.args.size())
        {
            std::cerr << "INTERNAL: No args to SHOW." << std::endl;
            throw 0;
        }

        for(auto& arg : node.args)
        {
            if(arg.args.size() != 2)
            {
                std::cerr << "INTERNAL: Not enough args to AS." << std::endl;
                throw 0;
            }

            load_args.push_back(as_t<identitifer_t>(arg));
        }
        tables = &tables_;
    }

    void run() override
    {
        for(auto& load_arg : load_args)
        {
            auto csv = load_arg.value.id;
            auto table_name = load_arg.name;

            if(tables->find(table_name) != tables->end())
            {
                std::cerr << "Invalid input: Attempted to load more than one"
                          << "table of the same name.    " << table_name << std::endl;
                throw 0;
            }

            tables->emplace(std::make_pair(table_name, std::make_shared<table>(csv)));
        }
    }
};
#endif

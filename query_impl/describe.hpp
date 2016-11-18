#ifndef _DESCRIBE_H
#define _DESCRIBE_H

#include <vector>

#include "../parser.hpp"
#include "../table.hpp"

#include "identitifer.hpp"
#include "query_object.hpp"

struct describe_t : query_object
{
    table_map_t* tables;
    std::vector<identitifer_t> table_identitifers;

    describe_t() = default;
    describe_t(parse_tree_node node,
             table_map_t& tables_)
    {
        if(!node.args.size())
        {
            std::cerr << "INTERNAL: No args to DESCRIBE." << std::endl;
            throw 0;
        }

        for(auto& arg : node.args)
        {
            table_identitifers.push_back(identitifer_t(arg));
        }

        tables = &tables_;
    }

    void run() override
    {
        for(auto& t : table_identitifers)
        {
            auto table = tables->find(t.id);
            if(table != tables->end())
            {
                table->second.describe();
            }
            else
            {
                std::cerr << "Could not resolve table name " << t.id << "." << std::endl;
                throw 0;
            }
        }

    }
};
#endif

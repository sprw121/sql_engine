#ifndef _SQL_ENGINE_H
#define _SQL_ENGINE

#include <algorithm>
#include <cstring>
#include <chrono>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

#include "parser.hpp"
#include "table.hpp"
#include "table_views.hpp"
#include "util.hpp"

#include "query_impl/compile.hpp"
#include "query_impl/query_object.hpp"

struct sql_engine
{
    table_map_t tables;
    std::vector<token_t> tokens;

    sql_engine() : tables() {};

    void run_shell()
    {
        while(1)
        {
            std::string line;
            std::cout << "> ";
            std::getline(std::cin, line);
            process_line(line);
        }
    }

    void process_line(std::string& line)
    {
        lexer l(line);
        token_t token;
        while(l.next_token(token))
        {
            output_token(token);
            switch(token.t)
            {
                case token_t::INVALID:
                    tokens = std::vector<token_t>();
                    break;
                case token_t::END:
                {
                    execute_query();
                    tokens.resize(0);
                    break;
                }
                default:
                    tokens.push_back(token);
                    break;
            }
        }
    }

    void load_from_csv(std::string& table_name, std::string& file_name)
    {
        if(tables.find(table_name) != tables.end())
        {
            std::cerr << "Invalid input: Attempted to load more than one table of the same name."
                      << "    " << table_name;
            throw 0;
        }

        tables.emplace(std::make_pair(table_name, std::make_shared<table>(file_name)));
    }

    void execute_query()
    {
        try
        {
            auto start = std::chrono::steady_clock::now();

            parse_tree_node p = parse(tokens);
            auto query = compile_query(p, tables);
            query->run();

            auto end = std::chrono::steady_clock::now();

            std::cerr << "Executed command in "
                      << std::chrono::duration<double, std::milli>(end - start).count()
                      << "ms." << std::endl;
        }
        catch(int) {}
    }
};

#endif

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
    std::unordered_map<std::string, table> tables;

    sql_engine() : tables() {};

    void run_shell()
    {
        std::vector<token_t> tokens;
        std::string line;
        token_t token;
        while(1)
        {
            std::cerr << "> ";
            std::getline(std::cin, line);
            lexer l(line);
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
                        execute_query(tokens);
                        tokens = std::vector<token_t>();
                        break;
                    }
                    default:
                        tokens.push_back(token);
                        break;
                }
            }
        }
    }

    void load_from_csv(const char* arg)
    {

        int idx = 0, len = strlen(arg);
        while(idx < len && arg[idx] != '=') idx++;
        std::string table_name(arg, idx), file_name(arg + idx + 1);

        if(tables.find(table_name) != tables.end())
        {
            std::cerr << "Invalid input: Attempted to load more than one table of the same name."
                      << "    " << table_name;
            throw 0;
        }

        tables.emplace(std::make_pair(table_name, table(file_name)));
    }

    void execute_query(std::vector<token_t>& tokens)
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
        catch(...) {}
    }
};

#endif

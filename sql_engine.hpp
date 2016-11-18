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
            std::cout << "> ";
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
/*
    void iterator_table(std::string table_name)
    {
        table_iterator view = table_iterator(&tables[table_name]);
        while(!view.empty())
        {
            for(unsigned int i = 0; i < view.width(); i++)
            {
                cell c = view.access_column(i);
                std::cout << c <<" ";
            }
            std::cout << std::endl;
            view.advance_row();
        }
    }

    void iterator_inner_join()
    {
        table_iterator a(&tables["a"]), b(&tables["b"]);
        inner_join<long long int> join(&a, 0, &b, 0);
        while(!join.empty())
        {
            for(unsigned int i = 0; i < join.width(); i++)
            {
                cell c = join.access_column(i);
                std::cout << c << " ";
            }
            std::cout << std::endl;
            join.advance_row();
        }

    }

    void iterator_outer_join()
    {
        table_iterator a(&tables["a"]), b(&tables["b"]);
        outer_join<long long int> join(&a, 0, &b, 0);
        while(!join.empty())
        {
            for(unsigned int i = 0; i < join.width(); i++)
            {
                cell c = join.access_column(i);
                std::cout << c << " ";
            }
            std::cout << std::endl;
            join.advance_row();
        }

    }

    void iterator_left_outer_join()
    {
        table_iterator a(&tables["a"]), b(&tables["b"]);
        left_outer_join<long long int> join(&a, 0, &b, 0);
        while(!join.empty())
        {
            for(unsigned int i = 0; i < join.width(); i++)
            {
                cell c = join.access_column(i);
                std::cout << c << " ";
            }
            std::cout << std::endl;
            join.advance_row();
        }

    }

    void iterator_right_outer_join()
    {
        table_iterator a(&tables["a"]), b(&tables["b"]);
        right_outer_join<long long int> join(&a, 0, &b, 0);
        while(!join.empty())
        {
            for(unsigned int i = 0; i < join.width(); i++)
            {
                cell c = join.access_column(i);
                std::cout << c << " ";
            }
            std::cout << std::endl;
            join.advance_row();
        }

    }

    void iterator_cross_join()
    {
        table_iterator a(&tables["a"]), b(&tables["b"]);
        cross_join<long long int> join(&a, &b);
        while(!join.empty())
        {
            for(unsigned int i = 0; i < join.width(); i++)
            {
                cell c = join.access_column(i);
                std::cout << c << " ";
            }
            std::cout << std::endl;
            join.advance_row();
        }

    }*/

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
            parse_tree_node p = parse(tokens);
            auto query = compile_query(p, tables);
            query->run();
        }
        catch(...) {}
    }
};

#endif

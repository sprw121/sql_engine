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

#include "compile.hpp"
#include "parser.hpp"
#include "select.hpp"
#include "table.hpp"
#include "util.hpp"

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

    }

    void load_from_csv(const char* arg)
    {

        int idx = 0, len = strlen(arg);
        while(idx < len && arg[idx] != '=') idx++;
        std::string table_name(arg, idx), file_name(arg + idx + 1);

        if(tables.find(table_name) != tables.end())
        {
            std::cerr << "Invalid input: Attempted to more than one table of the same name."
                      << "    " << table_name;
            throw 0;
        }

        load_from_csv(table_name, file_name);
    }

    void load_from_csv(std::string table_name, std::string file_name)
    {
        auto start = std::chrono::steady_clock::now();

        tables.emplace(std::make_pair(table_name, table(file_name)));

        auto end = std::chrono::steady_clock::now();
        std::cout << "Loaded FILE " << file_name << " as TABLE " << table_name << " : "
                  << std::chrono::duration<double, std::milli>(end - start).count()
                  << "ms." << std::endl;
    }

    void show(std::queue<token_t>& tokens)
    {
        token_t token = pop_front(tokens);
        if(tokens.size() || token.t != token_t::TABLES)
        {
            std::cerr << "Only clause TABLES is implemented for SHOW.";
            throw 0;
        }
        else
        {
            std::cout << std::endl;
            if(tables.size() == 0)
            {
                std::cout << "No tables loaded." << std::endl;
            }
            else
            {
                std::cout << "TABLE_NAMES" << std::endl
                          << "___________" << std::endl;
            }
            for(auto& table: tables)
            {
                std::cout << table.first << std::endl;
            }
            std::cout << std::endl;
        }
    }

    void describe(std::queue<token_t>& tokens)
    {
        token_t token = pop_front(tokens);
        if(tokens.size() || token.t != token_t::IDENTITIFER)
        {
            std::cerr << "DESCRIBE takes 1 identitifer (table name).";
            throw 0;
        }
        else
        {
            std::string table_name = boost::get<std::string>(token.u);

            std::cout << std::endl;
            if(tables.find(table_name) != tables.end())
            {
                tables[table_name].describe();
            }
            else
            {
                std::cout << "No table named : " << table_name << ".";
            }
            std::cout << std::endl;
        }
    }

    void load(std::queue<token_t>& tokens)
    {
        token_t file_token = pop_front(tokens);
        token_t as = pop_front(tokens);
        token_t table_token = pop_front(tokens);

        if(file_token.t == token_t::IDENTITIFER &&
           as.t == token_t::AS &&
           table_token.t == token_t::IDENTITIFER)
        {
            load_from_csv(boost::get<std::string>(table_token.u),
                          boost::get<std::string>(file_token.u));
        }
        else
        {
            std::cerr << "LOAD usage: LOAD file.csv AS table;";
        }
    }

    void exit_engine(std::queue<token_t>& tokens)
    {
        if(tokens.size() != 0)
        {
            std::cerr << "Exit expects no trailing symbols.";
        }
        else
        {
            exit(1);
        }
    }

    void execute_query(std::vector<token_t>& tokens)
    {
        try
        {
            parse_tree_node p = parse(tokens);
            auto query_obejct = compile_query(p, tables);
        }
        catch(...) {}
    }
};

#endif

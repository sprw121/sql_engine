#include <cstdio>
#include <iostream>

#include "sql_engine.hpp"

int main(int argc, char** argv)
{

    if(argc < 2)
    {
        printf("Usage: ./csvsql FILE1_NAME FILE2_NAME... [(--execute query)]\n");
    }

    sql_engine engine;

    for(int arg_idx = 1;
        arg_idx < argc && argv[arg_idx][0] != '-';
        arg_idx++)
    {
        engine.load_from_csv(argv[arg_idx]);
    }

    engine.run_shell();

    return 0;
}

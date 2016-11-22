#include <cstdio>
#include <iostream>

#include "output_format.hpp"
#include "sql_engine.hpp"

format_t out_format = format_t::CSV;

void usage()
{
    printf("Usage: ./csvsql TABLE1=FILE_NAME1 TABLE2=FILE_NAME2... [(--execute query)]\n");
    exit(1);
}

int main(int argc, char** argv)
{
    sql_engine engine;

    int arg_idx = 1;
    for(arg_idx;
        arg_idx < argc && argv[arg_idx][0] != '-';
        arg_idx++)
    {
        auto arg = argv[arg_idx];
        int idx = 0, len = strlen(arg);
        while(idx < len && arg[idx] != '=') idx++;
        if(idx == len)
            usage();

        std::string table_name(arg, idx), file_name(arg + idx + 1);
        engine.load_from_csv(table_name, file_name);
    }

    if(arg_idx == argc)
    {
        engine.run_shell();
    }
    else if(arg_idx == argc - 2)
    {
        std::string command(argv[arg_idx]);
        std::string command_arg(argv[arg_idx+1]);

        if(command == "--execute")
        {
            out_format = format_t::CSV;
            engine.process_line(command_arg);
        }
        else
            usage();
    }
    else
    {
        usage();
    }

    return 0;
}

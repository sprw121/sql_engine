#include <chrono>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>

#include "parse_csv.hpp"
#include "table.hpp"
#include "table_views.hpp"

// Reconstruct a table from a file
table::table(std::string& file_name)
{
    std::vector<std::vector<char*>> ir = parse_csv(file_name);

    for(auto name: ir[0])
    {
        column_names.push_back(std::string(name));
    }

    column_types = infer_column_types(ir);
    cells        = load_from_ir(ir, column_types);
    width        = cells.size();
    height       = cells[0].size();
}

// Called from table_view if we want to load
// a non-trivial table_view from (join, select) into memory,
// as a concrete table.

// The only use case that requires this currently is for non-trivial joins.
table::table(table_view& view)
{
    cells = std::vector<std::vector<cell>>(view.width(),
                                           std::vector<cell>(view.height()));
    int curr_row = 0;
    while(!view.empty())
    {
        for(unsigned int i = 0; i < view.width(); i++)
        {
            cells[i][curr_row] = view.access_column(i);
        }
        view.advance_row();
        curr_row++;
    }
    for(auto& column : cells)
    {
        column.resize(curr_row);
    }

    column_names = view.column_names;
    column_types = view.column_types;

    width        = cells.size();
    height       = cells[0].size();
}

void table::describe()
{
    std::cout << std::setw(15) << std::left << "Column" << " | "
        << std::setw(15) << std::left << "Type"   << std::endl;
    std::cout << std::string(16, '-') << "+" << std::string(16, '-') << std::endl;
    for(unsigned int i = 0; i < column_names.size(); i++)
    {
        std::cout << std::setw(15) << column_names[i] << " | ";
        switch(column_types[i])
        {
            case cell_type::INT:
                std::cout << std::setw(15) << std::left << "long long int" << std::endl;
                break;
            case cell_type::FLOAT:
                std::cout << std::setw(15) << std::left << "double" << std::endl;
                break;
            default: break;
        }
    }

    std::cout << std::endl << std::endl;
}


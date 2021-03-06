#ifndef _TABLE_H
#define _TABLE_H

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

enum cell_type
{
    INT,
    FLOAT,
};

// Unsafe support for multi data types.
// Must check the column the cell is coming
// from in order to access it correctly
// Also, must do type inference in the
// expression compilation.
struct cell
{
    union
    {
        long long int i;
        double        d;
    };

    cell()                  : i(0) {};
    cell(long long int val) : i(val) {};
    cell(double val)        : d(val) {};
};

struct table_view;

struct table
{
    std::vector<std::string>        column_names;
    std::vector<cell_type>          column_types;
    std::vector<std::vector<cell>>  cells;
    unsigned int                    width, height;

    table() = default;
    table(std::string& file_name);
    table(table_view& view);
    void describe();
};

typedef std::unordered_map<std::string, std::shared_ptr<table>> table_map_t;

#endif

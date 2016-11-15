#ifndef _TABLE_H
#define _TABLE_H

#include <string>
#include <vector>

#include "boost/variant.hpp"
#include "boost/variant/get.hpp"

enum type
{
    NUL,
    INT,
    FLOAT,
    STRING
};

typedef boost::variant<boost::blank, double, long long int, std::string> cell;

struct table
{
    std::vector<std::string>        column_names;
    std::vector<type>               column_types;
    std::vector<std::vector<cell>>  cells;
    unsigned int                    width, height;

    table() : column_names(std::vector<std::string>()),
              column_types(std::vector<type>()),
              cells(std::vector<std::vector<cell>>()),
              width(0), height(0) {};
    table(std::string& file_name);
    void describe();
};

#endif

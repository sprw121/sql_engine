#ifndef _TABLE_H
#define _TABLE_H

#include <cstdint>
#include <string>
#include <unordered_map>
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

typedef boost::variant<double, long long int, std::string> cell;

struct table
{
    std::vector<std::string>        column_names;
    std::vector<type>               column_types;
    std::vector<std::vector<cell>>  cells;
    unsigned int                    width, height;

    table() = default;
    table(std::string& file_name);
    void describe();
};

typedef std::unordered_map<std::string, table> table_map_t;

#endif

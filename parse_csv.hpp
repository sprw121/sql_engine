#ifndef _PARSE_CSV_H
#define _PARSE_CSV_H

#include <string>
#include <vector>

#include "table.hpp"

std::vector<std::vector<char*>> parse_csv(std::string& file_name);

std::vector<cell_type> infer_column_types(std::vector<std::vector<char*>> ir);

std::vector<std::vector<cell>> load_from_ir(std::vector<std::vector<char*>> ir,
                                            std::vector<cell_type> column_types);

#endif

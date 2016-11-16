#include <iostream>

#include "table.hpp"
#include "util.hpp"

// Convert a file into an vector of vector of c strings
std::vector<std::vector<char*>> parse_csv(std::string& file_name)
{
    FILE* fd = fopen(file_name.c_str(), "r");
    if(fd == NULL)
    {
        std::cerr << "Error opening file: " << file_name << std::endl;
        throw 0;
    }
    if(fseek(fd, 0, SEEK_END) != 0)
    {
        std::cerr << "Error reading file: " << file_name << std::endl;
        throw 0;
    }
    auto len = ftell(fd);
    rewind(fd);

    // A little hacky, but freeing this buffer is handling later
    // Due to the way the IR is constructed, ret[0][0] will hold this pointer
    char* buffer = (char*)malloc(sizeof(char) * (len + 1));
    if(buffer == NULL)
    {
        std::cerr << "OOM" << std::endl;
        exit(1);
    }

    fread(buffer, sizeof(char), len, fd);
    // Simplifies end of line logic
    if(buffer[len - 1] != '\n')
    {
        buffer[len] = '\n';
        len++;
    }
    fclose(fd);

    std::vector<std::vector<char*>> ret;

    // For now only hold pointers into our file buffer
    unsigned int begin = 0, columns = 0;
    std::vector<char*> tmp(0);
    for(unsigned int i = 0; i < len; i++)
    {
        char c = buffer[i];
        if(c == ',' || c == '\n')
        {
            buffer[i] = 0;
            tmp.push_back(&buffer[begin]);
            begin = i + 1;
        }

        if(c == '\n')
        {
            if(columns == 0) columns = tmp.size();
            if(tmp.size() != columns)
            {
                std::cout << "Error reading " << file_name
                    << ": mismatching column length on line " << ret.size() + 1
                    << ". Expected: " << columns
                    << ". Got: " << tmp.size() << std::endl;
                exit(1);
            }
            ret.push_back(tmp);
            tmp.resize(0);
        }
    }

    return ret;
}

type infer_type(const char* str)
{
    if(is_integer(str)) return type::INT;
    if(is_float(str)) return type::FLOAT;
    return type::STRING;
}

std::vector<type> infer_column_types(std::vector<std::vector<char*>> ir)
{
    std::vector<type> ret(ir[0].size(), type::INT);

    for(unsigned int i = 1; i < ir.size(); i++)
    {
        for(unsigned int j = 0; j < ir[i].size(); j++)
        {
            type t = infer_type(ir[i][j]);
            // Int columns containing a float get promoted to float column
            // Int or float columns containg a string get promoted to string
            switch(t)
            {
                case type::STRING:
                    ret[j] = type::STRING;
                    break;
                case type::FLOAT:
                    if(ret[j] == type::INT)
                    {
                        ret[j] = type::FLOAT;
                    }
                    break;
                default: break;
            }
        }
    }

    return ret;
}

// Converts our c string csv IR into a column-wise representation of boost::variants.
// Vector of columns.
std::vector<std::vector<cell>> load_from_ir(std::vector<std::vector<char*>> ir,
        std::vector<type> column_types)
{
    std::vector<std::vector<cell>> ret(ir[0].size(),
                                       std::vector<cell>(ir.size()-1));

    for(unsigned int i = 1; i < ir.size(); i++)
    {
        for(unsigned int j = 0; j < ir[i].size(); j++)
        {
            // Parse the c string representation and store in a boost::variant
            // Makes accessing contents later simpler, especially putting
            // values in maps for joins.
            switch(column_types[j])
            {
                case type::INT:
                    ret[j][i-1] = cell(atoll(ir[i][j]));
                    break;
                case type::FLOAT:
                    ret[j][i-1] = cell(atof(ir[i][j]));
                    break;
                case type::STRING:
                    ret[j][i-1] = cell(std::string(ir[i][j]));
                    break;
                default: break;
            }
        }
    }

    // See line 22
    free(ir[0][0]);
    return ret;
}

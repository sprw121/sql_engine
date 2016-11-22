#ifndef _TABLE_VIEWS_H
#define _TABLE_VIEWS_H

#include <memory>
#include <unordered_map>

#include "lexer.hpp"
#include "parser.hpp"
#include "table.hpp"

#include "query_impl/identitifer.hpp"
#include "query_impl/on.hpp"


// Interface to table-like structures.
// Provide basic iteration features
// and necessary information.


typedef std::unordered_map<long long int,
                           std::vector<unsigned int>> index_t;

struct table_iterator;

struct table_view
{
    std::string name;
    std::vector<std::string> column_names;
    std::vector<cell_type>   column_types;

    table_view() = default;
    virtual cell access_column(unsigned int i) = 0;
    virtual void advance_row() = 0;
    virtual bool empty() = 0;
    virtual unsigned int width() = 0;
    virtual unsigned int height() = 0;
    virtual std::shared_ptr<table_iterator> load() = 0;

    unsigned int resolve_column(std::string column_name)
    {
        if(name != "")
        {
            std::string qualified_column_name = name + "." + column_name;
            for(unsigned int i = 0; i < width(); i++)
            {
                if(column_names[i] == qualified_column_name)
                {
                    return i;
                }
            }
        }

        for(unsigned int i = 0; i < width(); i++)
        {
            if(column_names[i] == column_name)
            {
                return i;
            }
        }

        std::cerr << "Could not resolved column name: " << column_name << std::endl;
        throw 0;
    }
};

// Special view wraps a table.
struct table_iterator : table_view
{
    unsigned int            current_row;
    std::shared_ptr<table>  source;
    table_iterator(const table_iterator& other);
    table_iterator(table_view& view);
    table_iterator(parse_tree_node& node,
                   table_map_t& tables);

    cell access_column(unsigned int i) override;
    void advance_row() override;
    bool empty() override;
    unsigned int width() override;
    unsigned int height() override;
    std::shared_ptr<table_iterator> load();
    void reset();
};

struct view_factory
{
    std::shared_ptr<table_view> view;

    view_factory() = default;
    view_factory(parse_tree_node& node, table_map_t& tables);
};

#endif

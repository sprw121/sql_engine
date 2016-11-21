#ifndef _SELECT_H
#define _SELECT_H

#include <iomanip>
#include <memory>
#include <string>

#include "../parser.hpp"
#include "../table.hpp"
#include "../table_views.hpp"

#include "from.hpp"
#include "limit.hpp"
#include "offset.hpp"
#include "query_object.hpp"
#include "where.hpp"

struct from_iterator
{
    from_t from;
    where_t where;
    limit_t limit;
    offset_t offset;

    from_iterator(from_t& from_,
                  parse_tree_node& where_node,
                  limit_t& limit_,
                  offset_t& offset_) : from(from_), where(where_node, from),
                                       limit(limit_), offset(offset_)
    {
        if(!where.filter())
        {
            while(!from.view->empty() && !where.filter())
            {
                from.view->advance_row();
            }
        }
        while(!from.view->empty() && offset.offset--)
        {
            from.view->advance_row();
            while(!from.view->empty() && !where.filter())
            {
                from.view->advance_row();
            }
        }
    }

    cell access_column(unsigned int i)
    {
        return from.view->access_column(i);
    }

    void advance_row()
    {
        limit.limit--;
        from.view->advance_row();
        while(!from.view->empty() && !where.filter())
            from.view->advance_row();
    }

    bool empty()
    {

        return (limit.limit == 0) || (from.view->empty());
    }

    unsigned int width()
    {
        return from.view->width();
    }

    unsigned int height()
    {
        if(from.view->height() - offset.offset < 0)
            return 0;
        else
            return from.view->height() - offset.offset < limit.limit ?
                                      from.view->height() : limit.limit;
    }
};

struct select_t : query_object, table_view
{
    from_iterator it;
    select_t() = default;
    select_t(from_t& from, parse_tree_node where, limit_t& limit, offset_t& offset) :
        it(from, where, limit, offset) {};

    std::shared_ptr<table_iterator> load() override
    {
        return std::make_shared<table_iterator>(*this);
    }

    void run() override
    {
        auto num_columns = width();
        std::cout << height() << std::endl;
        for(auto& col_name : column_names)
            std::cout << std::setw(10) << col_name << " | ";
        std::cout << std::endl;

        for(unsigned int i = 0 ; i < num_columns; i++)
            std::cout << "-----------+-";
        std::cout << std::endl;


        while(!empty())
        {
            for(unsigned int i = 0; i < num_columns; i++)
            {
                cell value = access_column(i);
                if(column_types[i] == cell_type::INT)
                    std::cout << std::setw(10) << value.i << " | ";
                else
                    std::cout << std::setw(10) << value.d << " | ";
            }
            std::cout << std::endl;
            advance_row();
        }
    }
};

std::unique_ptr<select_t> select_factory(parse_tree_node&, table_map_t&);
#endif

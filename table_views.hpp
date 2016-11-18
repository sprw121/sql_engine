#ifndef _TABLE_VIEWS_H
#define _TABLE_VIEWS_H

#include <unordered_map>

#include "lexer.hpp"
#include "parser.hpp"
#include "table.hpp"

//#include "boost/variant/get.hpp"

#include "query_impl/identitifer.hpp"

struct table_view
{
    std::string name;

    virtual cell access_column(unsigned int i) = 0;
    virtual void advance_row() = 0;
    virtual bool empty() = 0;
    virtual unsigned int width() = 0;
    virtual unsigned int height() = 0;
    virtual unsigned int resolve_column(std::string) { return 0; }
};

struct table_iterator : table_view
{
    unsigned int current_row;
    table*       source;

    table_iterator(parse_tree_node& node,
                   table_map_t& tables)
    {
        current_row = 0;
        auto id = identitifer_t(node);
        auto table = tables.find(id.id);
        if(table != tables.end())
        {
            source = &table->second;
        }
        else
        {
            std::cerr << "Could not resolve table " << id.id << std::endl;
            throw 0;
        }
    }

    cell access_column(unsigned int i) override
    {
        return source->cells[i][current_row];
    }

    void advance_row() override
    {
        current_row++;
    }

    void reset()
    {
        current_row = 0;
    }

    bool empty() override
    {
        return current_row == source->height;
    }

    unsigned int width() override
    {
        return source->width;
    }

    unsigned int height() override
    {
        return source->height;
    }

    unsigned int resolve_column(std::string name) override
    {
        for(unsigned int i = 0; i < width(); i++)
        {
            if(source->column_names[i] == name)
            {
                return i;
            }
        }

        std::cerr << "Could not resolved column name: " << name << std::endl;
        throw 0;
    }
};

/*
template<typename T>
struct inner_join : table_view
{
    table_iterator* left;
    table_iterator* right;
    int left_column, right_column;
    std::unordered_map<T, int> index;

    inner_join(parse_tree_node node,
               table_map_t& tables)
    {
        if(left_->height() < right_->height())
        {
            left = left_;
            right = right_;
            left_column = left_column_;
            right_column = right_column_;
        }
        else
        {
            left = right_;
            right = left_;
            left_column = right_column_;
            right_column = left_column_;;
        }

        for(unsigned int i = 0; i < left->height(); i++)
        {
            index[boost::get<T>(left->source->cells[left_column][i])] = i;
        }

        while(!right->empty())
        {
            if(index.find(boost::get<T>(right->access_column(right_column))) != index.end())
            {
                break;
            }
            right->advance_row();
        }
    }

    cell access_column(unsigned int i) override
    {
        if(i >= left->width())
        {
            return right->access_column(i - left->width());
        }
        else
        {
            return left->source->cells[i][index[boost::get<T>(right->access_column(right_column))]];
        }
    }

    void advance_row() override
    {
        right->advance_row();
        while(!right->empty())
        {
            if(index.find(boost::get<T>(right->access_column(right_column))) != index.end())
            {
                break;
            }
            right->advance_row();
        }
    }

    bool empty() override
    {
        return index.size() == 0 || right->empty();
    }
    unsigned int width() override
    {
        return left->width() + right->width();
    }
    unsigned int height() override
    {
        return left->height() > right->height() ? right->height() : left->height();
    }
};

template<typename T>
struct outer_join : table_view
{
    table_iterator* left;
    table_iterator* right;
    int left_column, right_column;
    std::unordered_map<T, int> index;

    outer_join(parse_tree_node node,
               table_map_t& tables)
    {
        if(left_->height() < right_->height())
        {
            left = left_;
            right = right_;
            left_column = left_column_;
            right_column = right_column_;
        }
        else
        {
            left = right_;
            right = left_;
            left_column = right_column_;
            right_column = left_column_;;
        }

        for(unsigned int i = 0; i < left->height(); i++)
        {
            index[boost::get<T>(left->source->cells[left_column][i])] = i;
        }
        std::cout << index.size() << std::endl;
    }

    cell access_column(unsigned int i) override
    {
        if(i >= left->width())
        {
            return right->access_column(i - left->width());
        }
        else
        {
            return left->source->cells[i][index[boost::get<T>(right->access_column(right_column))]];
        }
    }

    void advance_row() override
    {
        while(!right->empty())
        {
            right->advance_row();
        }
    }

    bool empty() override
    {
        return index.size() == 0 || right->empty();
    }
    unsigned int width() override
    {
        return left->width() + right->width();
    }
    unsigned int height() override
    {
        return left->height() > right->height() ? right->height() : left->height();
    }
};

template<typename T>
struct left_outer_join : table_view
{
    table_iterator* left;
    table_iterator* right;
    int l_col, r_col;
    std::unordered_map<T, int> index;

    left_outer_join(parse_tree_node node
                    table_map_t tables)
    {
        left = left_;
        l_col = left_column_;
        right = right_;
        r_col = right_column_;

        for(unsigned int i = 0; i < right->height(); i++)
        {
            index[boost::get<T>(right->source->cells[r_col][i])] = i;
        }
    }

    cell access_column(unsigned int i) override
    {
        if(i >= left->width())
        {
            if(index.find(boost::get<T>(left->access_column(l_col))) != index.end())
            {
                return right->source->cells
                            [i-left->width()][index[boost::get<T>(left->access_column(l_col))]];
            }
            else
            {
                return cell();
            }
        }
        else
        {
            return left->access_column(i);
        }
    }

    void advance_row() override
    {
        left->advance_row();
    }

    bool empty() override
    {
        return index.size() == 0 || left->empty();
    }
    unsigned int width() override
    {
        return left->width() + right->width();
    }
    unsigned int height() override
    {
        return left->height();
    }
};

template<typename T> struct right_outer_join : table_view
{
    table_iterator* left;
    table_iterator* right;
    int l_col, r_col;
    std::unordered_map<T, int> index;

    right_outer_join(parse_tree_node node
                     table_map_t& tables)
    {
        left = left_;
        l_col = left_column_;
        right = right_;
        r_col = right_column_;

        for(unsigned int i = 0; i < right->height(); i++)
        {
            index[boost::get<T>(left->source->cells[l_col][i])] = i;
        }
    }

    cell access_column(unsigned int i) override
    {
        if(i >= left->width())
        {
            return right->access_column(i - left->width());
        }
        else
        {
            if(index.find(boost::get<T>(right->access_column(r_col))) != index.end())
            {
                return left->source->cells
                            [i][index[boost::get<T>(right->access_column(r_col))]];
            }
            else
            {
                return cell();
            }
        }
    }

    void advance_row() override
    {
        right->advance_row();
    }

    bool empty() override
    {
        return index.size() == 0 || right->empty();
    }
    unsigned int width() override
    {
        return left->width() + right->width();
    }
    unsigned int height() override
    {
        return right->height();
    }
};

template<typename T>
struct cross_join : table_view
{
    table_iterator* left;
    table_iterator* right;

    cross_join(parse_tree_node node,
                table_map_t tables)
    {
    }

    cell access_column(unsigned int i) override
    {
        if(i >= left->width())
        {
            return right->access_column(i - left->width());
        }
        else
        {
            return left->access_column(i);
        }
    }

    void advance_row() override
    {
        left->advance_row();
        if(left->empty())
        {
            left->reset();
            right->advance_row();
        }
    }

    bool empty() override
    {
        return right->empty();
    }
    unsigned int width() override
    {
        return left->width() + right->width();
    }
    unsigned int height() override
    {
        return left->height() * right->height();
    }
};*/

#endif

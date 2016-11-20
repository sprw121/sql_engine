#ifndef _TABLE_VIEWS_H
#define _TABLE_VIEWS_H

#include <memory>
#include <unordered_map>

#include "lexer.hpp"
#include "parser.hpp"
#include "table.hpp"

#include "boost/functional/hash.hpp"

#include "query_impl/identitifer.hpp"
#include "query_impl/on.hpp"


typedef std::unordered_map<cell,
                           std::vector<unsigned int>,
                           boost::hash<cell>> index_t;

struct table_iterator;

struct table_view
{
    std::string name;
    std::vector<std::string> column_names;
    std::vector<cell_type> column_types;

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

struct table_iterator : table_view
{
    unsigned int            current_row;
    std::shared_ptr<table>  source;

    table_iterator(const table_iterator& other) : table_view()
    {
        current_row = 0;
        name = other.name;
        source = other.source;
        column_names = other.column_names;
        column_types = other.column_types;
    }

    table_iterator(table_view& view) : table_view()
    {
        current_row = 0;
        source = std::make_shared<table>(view);
        name = view.name;
        column_names = source->column_names;
        column_types = source->column_types;
    }

    table_iterator(parse_tree_node& node,
                   table_map_t& tables) : table_view()
    {
        current_row = 0;
        auto id = identitifer_t(node);
        auto table = tables.find(id.id);
        if(table != tables.end())
        {
            source = table->second;
        }
        else
        {
            std::cerr << "Could not resolve table " << id.id << std::endl;
            throw 0;
        }
        name = id.id;
        column_names = table->second->column_names;
        column_types = table->second->column_types;
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

    std::shared_ptr<table_iterator> load() override
    {
        return std::shared_ptr<table_iterator>(new table_iterator(*this));
    }
};

struct indexed_join : table_view
{
    enum index_side
    {
        LEFT,
        RIGHT,
        HEIGHT
    };

    std::shared_ptr<table_iterator> indexed_side;
    std::shared_ptr<table_iterator> iterator_side;
    int iterator_column, indexed_column;

    std::shared_ptr<table_iterator> left;
    std::shared_ptr<table_iterator> right;
    int left_column, right_column;

    index_t index;
    index_side side;

    // Dummy to initialize iterator variables
    std::vector<unsigned int> empty_vector;
    std::vector<unsigned int>::iterator index_cache;
    std::vector<unsigned int>::iterator empty_cache;

    indexed_join(std::shared_ptr<table_iterator> left_,
                 std::shared_ptr<table_iterator> right_,
                 on_t& on, index_side side_) : table_view(), left(left_),
                                               right(right_), side(side_)
    {
        left_column = left->resolve_column(on.left_id.id);
        right_column = right->resolve_column(on.right_id.id);
        if(side_ == HEIGHT)
        {
            side = left->height() > right->height() ? RIGHT : LEFT;
        }
        else
        {
            side = side_;
        }

        if(side == LEFT)
        {
            indexed_side = left;
            indexed_column = left_column;
            iterator_side = right;
            iterator_column = right_column;
        }
        else
        {
            indexed_side = right;
            indexed_column = right_column;
            iterator_side = left;
            iterator_column = left_column;
        }

        for(unsigned int i = 0; i < indexed_side->height(); i++)
        {
            auto index_cell = indexed_side->source->cells[indexed_column][i];
            auto found = index.find(index_cell);
            if(found == index.end())
            {
                index.emplace(make_pair(index_cell, std::vector<unsigned int>{i}));
            }
            else
            {
                found->second.push_back(i);
            }
        }

        column_types.insert(column_types.end(),
                            left->column_types.begin(),
                            left->column_types.end());
        column_types.insert(column_types.end(),
                            right->column_types.begin(),
                            right->column_types.end());

        for(auto& col_name : left->column_names)
        {
             if(left->name != "")
                column_names.push_back(left->name + "." + col_name);
            else
                column_names.push_back(col_name);

        }
        for(auto& col_name : right->column_names)
        {
             if(right->name != "")
                column_names.push_back(right->name + "." + col_name);
            else
                column_names.push_back(col_name);

        }

        auto found = index.find(iterator_side->access_column(iterator_column));
        if(found != index.end())
        {
            index_cache = found->second.begin();
            empty_cache = found->second.end();
        }
        else
        {
            index_cache = empty_vector.end();
            empty_cache = empty_vector.end();
        }
    }

    std::shared_ptr<table_iterator> load()
    {
        return std::make_shared<table_iterator>(*this);
    }
};

struct inner_join : indexed_join
{
    inner_join(std::shared_ptr<table_iterator> left_,
               std::shared_ptr<table_iterator> right_,
               on_t on) : indexed_join(left_, right_, on, HEIGHT)
    {
        while(!iterator_side->empty())
        {
            auto found = index.find(iterator_side->access_column(iterator_column));
            if(found != index.end())
            {
                index_cache = found->second.begin();
                empty_cache   = found->second.end();
                break;
            }
            iterator_side->advance_row();
        }
    }

    cell access_column(unsigned int i) override
    {
        if(i >= left->width())
            if(side == LEFT)
                return right->access_column(i - left->width());
            else
                return right->source->cells[i - left->width()][*index_cache];
        else
            if(side == LEFT)
                return left->source->cells[i][*index_cache];
            else
                return left->access_column(i);
    }

    void advance_row() override
    {
        if(++index_cache == empty_cache)
        {
            iterator_side->advance_row();
            while(!iterator_side->empty())
            {
                auto found = index.find(iterator_side->access_column(iterator_column));
                if(found != index.end())
                {
                    index_cache = found->second.begin();
                    empty_cache   = found->second.end();
                    break;
                }
                iterator_side->advance_row();
            }
        }
    }

    bool empty() override
    {
        return index.size() == 0 || iterator_side->empty();
    }

    unsigned int width() override
    {
        return iterator_side->width() + indexed_side->width();
    }

    // Speculative
    unsigned int height() override
    {
        return indexed_side->height() > iterator_side->height() ?
                    iterator_side->height() : indexed_side->height();
    }
};

struct outer_join : indexed_join
{
    std::vector<bool> visited;
    unsigned int next_unvisited = 0;
    outer_join(std::shared_ptr<table_iterator> left_,
               std::shared_ptr<table_iterator> right_,
               on_t& on) : indexed_join(left_, right_, on, HEIGHT)
    {
        if(side == LEFT)
        {
            visited = std::vector<bool>(left->height());
        }
        else
        {
            visited = std::vector<bool>(right->height());
        }
    }

    cell access_column(unsigned int i) override
    {
        if(!iterator_side->empty())
            if(i >= left->width())
                if(side == LEFT)
                    return right->access_column(i - left->width());
                else
                {
                    if(index_cache != empty_cache)
                    {
                        visited[*index_cache] = true;
                        return right->source->cells[i - left->width()][*index_cache];
                    }
                    else
                        return cell();
                }
            else
                if(side == LEFT)
                {
                    if(index_cache != empty_cache)
                    {
                        visited[*index_cache] = true;
                        return left->source->cells[i][*index_cache];
                    }
                    else
                        return cell();
                }
                else
                    return left->access_column(i);
        else
            if(i >= left->width())
                if(side == LEFT)
                    return cell();
                else
                    return right->source->cells[i - left->width()][next_unvisited];
            else
                if(side == LEFT)
                    return left->source->cells[i][next_unvisited];
                else
                    return cell();
    }

    void advance_row() override
    {
        if(!iterator_side->empty())
        {
            if(index_cache == empty_cache || ++index_cache == empty_cache)
            {
                iterator_side->advance_row();
                if(!iterator_side->empty())
                {
                    auto found = index.find(iterator_side->access_column(iterator_column));
                    if(found != index.end())
                    {
                        index_cache = found->second.begin();
                        empty_cache = found->second.end();
                    }
                }
            }
        }
        else
        {
            next_unvisited++;
            while(next_unvisited < visited.size() &&
                  visited[next_unvisited])
            {
                next_unvisited++;
            }
        }
    }

    bool empty() override
    {
        return iterator_side->empty() && next_unvisited == indexed_side->height();
    }
    unsigned int width() override
    {
        return iterator_side->width() + indexed_side->width();
    }

    // Speculative
    unsigned int height() override
    {
        return iterator_side->height() + indexed_side->height();
    }
};

struct left_outer_join : indexed_join
{
    left_outer_join(std::shared_ptr<table_iterator> left_,
                    std::shared_ptr<table_iterator> right_,
                    on_t on) : indexed_join(left_, right_, on, RIGHT) {};

    cell access_column(unsigned int i) override
    {
        if(i >= left->width())
        {
            if(index_cache != empty_cache)
            {
                return right->source->cells[i - left->width()][*index_cache];
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
        if(index_cache == empty_cache || ++index_cache == empty_cache)
        {
            iterator_side->advance_row();
            if(!iterator_side->empty())
            {
                auto found = index.find(left->access_column(left_column));
                if(found != index.end())
                {
                    index_cache = found->second.begin();
                    empty_cache = found->second.end();
                }
            }
        }
    }

    bool empty() override
    {
        return index.size() == 0 || iterator_side->empty();
    }
    unsigned int width() override
    {
        return iterator_side->width() + indexed_side->width();
    }
    unsigned int height() override
    {
        return iterator_side->height();
    }
};

struct right_outer_join : indexed_join
{
    right_outer_join(std::shared_ptr<table_iterator> left_,
                     std::shared_ptr<table_iterator> right_,
                     on_t on) : indexed_join(left_, right_, on, LEFT) {};

    cell access_column(unsigned int i) override
    {
        if(i >= left->width())
        {
            return right->access_column(i - left->width());
        }
        else
        {
            if(index_cache == empty_cache)
            {
                return cell();
            }
            else
            {
                return left->source->cells[i][*index_cache];
            }
        }
    }

    void advance_row() override
    {
        if(index_cache == empty_cache || ++index_cache == empty_cache)
        {
            iterator_side->advance_row();
            if(!iterator_side->empty())
            {
                auto found = index.find(right->access_column(right_column));
                if(found != index.end())
                {
                    index_cache = found->second.begin();
                    empty_cache = found->second.end();
                }
            }
        }
    }

    bool empty() override
    {
        return index.size() == 0 || iterator_side->empty();
    }
    unsigned int width() override
    {
        return indexed_side->width() + iterator_side->width();
    }
    unsigned int height() override
    {
        return iterator_side->height();
    }
};

struct cross_join : table_view
{
    std::shared_ptr<table_iterator> left;
    std::shared_ptr<table_iterator> right;

    cross_join(std::shared_ptr<table_iterator> left_,
               std::shared_ptr<table_iterator> right_) : table_view(),
                                                         left(left_), right(right_)
    {
        for(auto& col_name : left->column_names)
        {
             if(left->name != "")
                column_names.push_back(left->name + "." + col_name);
            else
                column_names.push_back(col_name);

        }
        for(auto& col_name : right->column_names)
        {
             if(right->name != "")
                column_names.push_back(right->name + "." + col_name);
            else
                column_names.push_back(col_name);

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

    std::shared_ptr<table_iterator> load()
    {
        return std::make_shared<table_iterator>(*this);
    }
};

struct view_factory
{
    std::shared_ptr<table_view> view;

    view_factory(parse_tree_node node, table_map_t& tables)
    {
        switch(node.token.t)
        {
            case token_t::AS:
            {
                if(node.args.size() != 2)
                {
                    std::cerr << "INTERNAL: Not enough args to AS." << std::endl;
                    throw 0;
                }
            }
            case token_t::IDENTITIFER:
            {
                view = std::make_shared<table_iterator>(node, tables);
                break;
            }
            case token_t::OUTER_JOIN:
            case token_t::INNER_JOIN:
            case token_t::LEFT_JOIN:
            case token_t::RIGHT_JOIN:
            {
                if(node.args.size() != 3)
                {
                    std::cerr << "Not enough args to " << output_token(node.token)
                              << ". (perhaps ON required>)" << std::endl;
                    throw 0;
                }

                on_t on(node.args[0]);
                auto left_container     = view_factory(node.args[2], tables);
                auto left_view          = left_container.view;
                auto left_side          = left_view->load();

                auto right_container    = view_factory(node.args[1], tables);
                auto right_view         = right_container.view;
                auto right_side         = right_view->load();

                if(node.token.t == token_t::OUTER_JOIN)
                {
                    view = std::shared_ptr<table_view>(
                                new outer_join(left_side, right_side, on));
                }
                if(node.token.t == token_t::INNER_JOIN)
                {
                    view = std::shared_ptr<table_view>(
                                new inner_join(left_side, right_side, on));
                }
                if(node.token.t == token_t::LEFT_JOIN)
                {
                    view = std::shared_ptr<table_view>(
                                new left_outer_join(left_side, right_side, on));
                }
                if(node.token.t == token_t::RIGHT_JOIN)
                {
                    view = std::shared_ptr<table_view>(
                                new right_outer_join(left_side, right_side, on));
                }
                break;
            }
            case token_t::CROSS_JOIN:
            {
                if(node.args.size() != 2)
                {
                    std::cerr << "INTERNAL: Not enough args to CROSS_JOIN." << std::endl;
                    throw 0;
                }

                auto left_container     = view_factory(node.args[0], tables);
                auto left_view          = left_container.view;
                auto left_side          = left_view->load();

                auto right_container    = view_factory(node.args[1], tables);
                auto right_view         = right_container.view;
                auto right_side         = right_view->load();

                view = std::shared_ptr<table_view>(new cross_join(left_side, right_side));
                break;
            }
            default:
            {
                std::cerr << "Unexpect argument to FROM." << std::endl;
                throw 0;
            }
        }
    }
};

#endif

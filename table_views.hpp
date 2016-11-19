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

    virtual cell access_column(unsigned int i) = 0;
    virtual void advance_row() = 0;
    virtual bool empty() = 0;
    virtual unsigned int width() = 0;
    virtual unsigned int height() = 0;
    virtual std::shared_ptr<table_iterator> load() { };
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

struct inner_join : table_view
{
    std::shared_ptr<table_iterator> left;
    std::shared_ptr<table_iterator> right;
    int left_column, right_column;
    index_t index;

    inner_join(std::shared_ptr<table_iterator> left_,
               std::shared_ptr<table_iterator> right_,
               on_t on)
    {
        if(left_->height() < right_->height())
        {
            left = left_;
            right = right_;
        }
        else
        {
            left = right_;
            right = left_;
        }

        for(unsigned int i = 0; i < left->height(); i++)
        {
            auto index_cell = left->source->cells[left_column][i];
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

        while(!right->empty())
        {
            if(index.find(right->access_column(right_column)) != index.end())
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
            auto found = index.find(right->access_column(right_column));
            return left->source->cells[i][found->second[0]];
        }
    }

    void advance_row() override
    {
        right->advance_row();
        while(!right->empty())
        {
            if(index.find(right->access_column(right_column)) != index.end())
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

struct outer_join : table_view
{
    std::shared_ptr<table_iterator> left;
    std::shared_ptr<table_iterator> right;
    int left_column, right_column;
    index_t index;

    outer_join(std::shared_ptr<table_iterator> left_,
               std::shared_ptr<table_iterator> right_,
               on_t on)
    {
        if(left_->height() < right_->height())
        {
            left = left_;
            right = right_;
        }
        else
        {
            left = right_;
            right = left_;
        }

        for(unsigned int i = 0; i < left->height(); i++)
        {
            auto index_cell = left->source->cells[left_column][i];
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
    }

    cell access_column(unsigned int i) override
    {
        if(i >= left->width())
        {
            return right->access_column(i - left->width());
        }
        else
        {
            auto found = index.find(right->access_column(right_column));
            if(found != index.end())
            {
                return left->source->cells[i][found->second[0]];
            }
            else
            {
                return cell();
            }
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

struct left_outer_join : table_view
{
    std::shared_ptr<table_iterator> left;
    std::shared_ptr<table_iterator> right;
    int l_col, r_col;
    index_t index;

    left_outer_join(std::shared_ptr<table_iterator> left_,
                    std::shared_ptr<table_iterator> right_,
                    on_t on)
    {
        left = left_;
        right = right_;

        for(unsigned int i = 0; i < left->height(); i++)
        {
            auto index_cell = right->source->cells[r_col][i];
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
    }

    cell access_column(unsigned int i) override
    {
        if(i >= left->width())
        {
            auto found = index.find(left->access_column(l_col));
            if(found != index.end())
            {
                return right->source->cells[i - left->width()][found->second[0]];
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

struct right_outer_join : table_view
{
    std::shared_ptr<table_iterator> left;
    std::shared_ptr<table_iterator> right;
    int l_col, r_col;
    index_t index;

    right_outer_join(std::shared_ptr<table_iterator> left_,
                     std::shared_ptr<table_iterator> right_,
                     on_t on)
    {
        left = left_;
        right = right_;

        for(unsigned int i = 0; i < left->height(); i++)
        {
            auto index_cell = left->source->cells[l_col][i];
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
    }

    cell access_column(unsigned int i) override
    {
        if(i >= left->width())
        {
            return right->access_column(i - left->width());
        }
        else
        {
            auto found = index.find(right->access_column(r_col));
            if(found != index.end())
            {
                return left->source->cells[i][found->second[0]];
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

struct cross_join : table_view
{
    std::shared_ptr<table_iterator> left;
    std::shared_ptr<table_iterator> right;

    cross_join(std::shared_ptr<table_iterator> left_,
               std::shared_ptr<table_iterator> right_) : left(left_), right(right_) {};

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
                    view = std::make_shared<outer_join>(left_side, right_side, on);
                }
                if(node.token.t == token_t::INNER_JOIN)
                {
                    view = std::make_shared<inner_join>(left_side, right_side, on);
                }
                if(node.token.t == token_t::LEFT_JOIN)
                {
                    view = std::make_shared<left_outer_join>(left_side, right_side, on);
                }
                if(node.token.t == token_t::RIGHT_JOIN)
                {
                    view = std::make_shared<right_outer_join>(left_side, right_side, on);
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

                view = std::make_shared<cross_join>(left_side, right_side);
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
/*
std::shared_ptr<table_view> view_factory(parse_tree_node node, table_map_t& tables)
{
    std::shared_ptr<table_view> view;

    return view;
};*/
#endif


#include "table_views.hpp"

#include "query_impl/as.hpp"
#include "query_impl/select.hpp"

table_iterator::table_iterator(const table_iterator& other) : table_view()
{
    current_row = 0;
    name = other.name;
    source = other.source;
    column_names = other.column_names;
    column_types = other.column_types;
}

// Joins want to have table_iterators for row-based access.
// Provide an interface for them.
table_iterator::table_iterator(table_view& view) : table_view()
{
    current_row = 0;
    source = std::make_shared<table>(view);
    name = view.name;
    column_names = source->column_names;
    column_types = source->column_types;
}

table_iterator::table_iterator(parse_tree_node& node,
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

cell table_iterator::access_column(unsigned int i)
{
    return source->cells[i][current_row];
}

void table_iterator::advance_row()
{
    current_row++;
}

void table_iterator::reset()
{
    current_row = 0;
}

bool table_iterator::empty()
{
    return current_row == source->height;
}

unsigned int table_iterator::width()
{
    return source->width;
}

unsigned int table_iterator::height()
{
    return source->height;
}

// Would be nice to return a pointer to this, can't
// because them our reference count would exist in
// two places.
std::shared_ptr<table_iterator> table_iterator::load()
{
    return std::shared_ptr<table_iterator>(new table_iterator(*this));
}

// Base class for left, right, outer, inner joins
// that deals with the construction of an index.

// Indexed joins allow for joining on non-unique
// columns by stores a map key -> vector<index>,
// Lookups then cache an iterator to this vector,
// which we then iterator over.
struct indexed_join : table_view
{
    // Refers to side indexed
    enum index_side
    {
        LEFT,
        RIGHT,
        HEIGHT
    };

    std::shared_ptr<table_iterator> indexed_side;
    std::shared_ptr<table_iterator> iterator_side;
    int iterator_column, indexed_column;

    // Hold addtion pointers for access in
    // certain places in derived classes.
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

        // Can only join on int columns, doesn't really make
        // sense to join on floats?
        if(left->column_types[left_column] != cell_type::INT)
        {
            std::cerr << "Joining on non-integer column not supported in "
                      << "left side of join." << std::endl;
            throw 0;
        }

        if(right->column_types[right_column] != cell_type::INT)
        {
            std::cerr << "Joining on non-integer column not supported in "
                      << "right side of join." << std::endl;
            throw 0;
        }

        // If we can, index the smaller side
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
            auto found = index.find(index_cell.i);
            if(found == index.end())
            {
                index.emplace(make_pair(index_cell.i, std::vector<unsigned int>{i}));
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

        auto found = index.find(iterator_side->access_column(iterator_column).i);
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

// Index the smaller side, iterate over the larger side
// and until we find rows that match with the index.
struct inner_join : indexed_join
{
    inner_join(std::shared_ptr<table_iterator> left_,
               std::shared_ptr<table_iterator> right_,
               on_t on) : indexed_join(left_, right_, on, HEIGHT)
    {
        // Need to do first lookup in constructor.
        while(!iterator_side->empty())
        {
            auto found = index.find(iterator_side->access_column(iterator_column).i);
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
        // If the last lookup has run out of indicies
        if(++index_cache == empty_cache)
        {
            // Find the next row with that matchs on the index
            iterator_side->advance_row();
            while(!iterator_side->empty())
            {
                auto found = index.find(iterator_side->access_column(iterator_column).i);
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

// Index the smaller side, iterate over the larger side,
// if we have a index match, iterator over the matching rows.
// If you don't match, just produce null columns. NULL
// is not supported by cell, so these just get 0 values.
// Store which rows we see, so when we iterate over the indexed side
// we can skip them.
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
        if(!iterator_side->empty()) // We're still iterating over the non-indexed side
            if(i >= left->width()) // Right column lookup
                if(side == LEFT) // Right side is iterator
                    return right->access_column(i - left->width());
                else // Left side is iterator, try to lookup
                {
                    if(index_cache != empty_cache)
                    {
                        visited[*index_cache] = true;
                        return right->source->cells[i - left->width()][*index_cache];
                    }
                    else
                        return cell();
                }
            else  // Left column lookup
                if(side == LEFT) // Left side is indexe
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
        else // If we're iterating over the indexed_side
            if(i >= left->width()) // Right column lookup
                if(side == LEFT)
                    return cell();
                else // Right side was indexed, and we're iterating over the indexed_side
                    return right->source->cells[i - left->width()][next_unvisited];
            else // Left column lookup
                if(side == LEFT) // Left side was indexed, and we're iterating over the indexed_side
                    return left->source->cells[i][next_unvisited];
                else
                    return cell();
    }

    void advance_row() override
    {
        if(!iterator_side->empty())
        {
            // If we don't have any match rows to iterate over
            if(index_cache == empty_cache || ++index_cache == empty_cache)
            {
                iterator_side->advance_row();
                if(!iterator_side->empty())
                {
                    auto found = index.find(iterator_side->access_column(iterator_column).i);
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
            // Skip ones we visited
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
                auto found = index.find(left->access_column(left_column).i);
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
                auto found = index.find(right->access_column(right_column).i);
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

// Relatively trivial join
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

view_factory::view_factory(parse_tree_node& node, table_map_t& tables)
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
            as_t<view_factory> a(node, tables);
            view = a.value.view;
            view->name = a.name;
            break;
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
        case token_t::SELECT:
        {
            view = select_factory(node, tables);
            break;
        }
        default:
        {
            std::cerr << "Unexpected argument to FROM." << std::endl;
            throw 0;
        }
    }
}

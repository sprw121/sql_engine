#include <iostream>
#include <sstream>
#include <stack>

#include "select.hpp"
#include "../util.hpp"

#include "aggregators.hpp"
#include "as.hpp"
#include "expression.hpp"

// Column select only take expressions in as
// our output columns. Iterates over the
// rows and generates column by executing the
// series of expressions on the current row.
struct column_select : select_t
{
    std::vector<std::unique_ptr<expression_t>> columns;

    column_select(from_t& from,
                  parse_tree_node& where_node,
                  limit_t& limit,
                  offset_t& offset,
                  std::stack<parse_tree_node>&expression_stack) :
                                select_t(from, where_node, limit, offset)
    {
        int column_idx = 0;
        while(!expression_stack.empty())
        {
            parse_tree_node arg = pop_top(expression_stack);
            if(arg.token.t == token_t::AS)
            {
                as_t<expression_container> named_expression(arg, from);
                column_names.push_back(named_expression.name);
                columns.emplace_back(std::move(named_expression.value.expression));
                column_types.push_back(columns.back()->return_type);
                column_idx++;
            }
            else if (arg.token.t == token_t::IDENTITIFER)
            {
                column_names.push_back(arg.token.raw_rep);
                columns.emplace_back(expression_factory(arg, from));
                column_types.push_back(columns.back()->return_type);
                column_idx++;
            }
            else if (arg.token.t == token_t::SELECT_ALL)
            {
                for(auto& column_name : from.view->column_names)
                {
                    columns.emplace_back(expression_factory(column_name, from));
                    column_names.push_back(column_name);
                    column_types.push_back(columns.back()->return_type);
                }
                column_idx += column_names.size();
            }
            else
            {
                std::stringstream stream;
                stream << "col_" << column_idx;
                columns.emplace_back(expression_factory(arg, from));
                column_names.push_back(stream.str());
                column_types.push_back(columns.back()->return_type);
                column_idx++;
            }
        }
    }

    cell access_column(unsigned int i) override
    {
        return columns[i]->call();
    }

    void advance_row() override
    {
        it.advance_row();
    }

    bool empty() override
    {
        return it.empty();
    }

    unsigned int width() override
    {
        return columns.size();
    }

    unsigned int height() override
    {
        return it.height();
    }
};

// Aggregate selects also from a table view,
// but only 1 row high, with the row being the
// values of the accumulators
struct aggregate_select : select_t
{
    std::vector<std::unique_ptr<aggregator_t>> columns;
    bool visited = false;

    aggregate_select(from_t& from,
                     parse_tree_node& where_node,
                     limit_t& limit,
                     offset_t& offset,
                     std::stack<parse_tree_node>&expression_stack) :
                                select_t(from, where_node, limit, offset)
    {
        int column_idx = 0;
        while(!expression_stack.empty())
        {
            parse_tree_node arg = pop_top(expression_stack);
            if(arg.token.t == token_t::AS)
            {
                as_t<aggregator_container> named_aggregator(arg, from);
                column_names.push_back(named_aggregator.name);
                columns.emplace_back(std::move(named_aggregator.value.aggregator));
                column_types.push_back(columns.back()->return_type);
                column_idx++;
            }
            else
            {
                std::stringstream stream;
                stream << "col_" << column_idx;
                columns.emplace_back(aggregator_factory(arg, from));
                column_names.push_back(stream.str());
                column_types.push_back(columns.back()->return_type);
                column_idx++;
            }
        }

        // Iterate over ourself, calls our aggregator expressions
        while(!it.empty())
        {
            for(auto& column : columns)
                column->accumulate();
            it.advance_row();
        }
    }

    cell access_column(unsigned int i) override
    {
        return columns[i]->value();
    }

    void advance_row() override
    {
        visited = true;
    }

    bool empty() override
    {
        return visited;
    }

    unsigned int width() override
    {
        return columns.size();
    }

    unsigned int height() override
    {
        return 1;
    }
};

// Logic for constructing a select is somewhat involved,
// And the type of select we're doing isn't known until
// we see the expressions that our generating the columns.
// Thus we need a factory method that calls the appropriate constructor
// Parser constructs the arguments in reverse order.
std::unique_ptr<select_t> select_factory(parse_tree_node& node,
                                         table_map_t& tables)
{
    if(node.args.size() < 2)
    {
        std::cerr << "Not enough args to select." << std::endl;
        throw 0;
    }

    from_t from;
    limit_t limit;
    offset_t offset;
    bool seen_offset = false, seen_limit = false, seen_where = false;
    parse_tree_node where_node;
    unsigned int i = 0;

    // Unpack all our possible clauses,
    // Check the validity of each clause with respect to what we've
    // already processed
    for(auto& arg : node.args)
    {
        switch(arg.token.t)
        {
            case token_t::OFFSET:
            {
                if(seen_offset || seen_where || seen_limit)
                {
                    std::cerr << "Unexpected OFFSET clause." << std::endl;
                    throw 0;
                }
                offset = offset_t(arg);
                seen_offset = true;
                break;
            }
            case token_t::LIMIT:
            {
                if(seen_limit || seen_where)
                {
                    std::cerr << "Unexpected LIMIT clause." << std::endl;
                    throw 0;
                }
                limit = limit_t(arg);
                seen_limit = true;
                break;
            }
            case token_t::WHERE:
            {
                if(seen_where)
                {
                    std::cerr << "Unexpected WHERE clause." << std::endl;
                    throw 0;
                }
                // Defer processing of where until we have a FROM.
                where_node = arg;
                seen_where = true;
                break;
            }
            case token_t::FROM:
            {
                from = from_t(arg, tables);
                goto end_loop; // End loop after we process our FROM clause
            }
            default:
            {
                std::cerr << "Unexpected clause after FROM." << std::endl;
                throw 0;
            }
        }
        i++;
    }
    end_loop:
    if(i++ == node.args.size())
    {
        std::cerr << "No FROM clause seen." << std::endl;
        throw 0;
    }
    if(i == node.args.size())
    {
        std::cerr << "No expressions passed to SELECT." << std::endl;
        throw 0;
    }
    if(seen_offset && !seen_limit)
    {
        std::cerr << "OFFSET clause without LIMIT clause." << std::endl;
        throw 0;
    }

    // Preprocess our column expression arguments.
    // Track whether we're doing an aggregate or column select
    // Arguments are in reverse order, so push them onto a stack
    // So we can processing them in order.
    bool seen_column_selector = false, seen_aggregator = false;
    std::stack<parse_tree_node> expression_stack;
    while(i < node.args.size())
    {
        if(node.args[i].token.t == token_t::FUNCTION)
            seen_aggregator = true;
        else if(node.args[i].token.t == token_t::AS)
        {
            if(node.args[i].args[0].token.t == token_t::FUNCTION)
                seen_aggregator = true;
            else
                seen_column_selector = true;
        }
        else
            seen_column_selector = true;

        if(seen_column_selector && seen_aggregator)
        {
            std::cerr << "Cannot select on column selection and aggregates." << std::endl;
            throw 0;
        }

        expression_stack.push(node.args[i++]);
    }

    // Dispath to appropciate select subtype
    if(seen_column_selector)
        return std::unique_ptr<select_t>(
            new column_select(from, where_node, limit, offset, expression_stack));
    else
        return std::unique_ptr<select_t>(
            new aggregate_select(from, where_node, limit, offset, expression_stack));
}

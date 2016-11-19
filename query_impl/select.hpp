struct select_t;

#ifndef _SELECT_H
#define _SELECT_H

#include <string>

#include "../parser.hpp"
#include "../table.hpp"
#include "../table_views.hpp"

#include "as.hpp"
#include "expression.hpp"
#include "from.hpp"
#include "limit.hpp"
#include "offset.hpp"
#include "query_object.hpp"
#include "where.hpp"

struct select_t : query_object, table_view
{
    std::vector<expression_t> columns;
    std::vector<std::string> column_names;
    std::vector<type> column_types;
    from_t from;
    where_t where;
    limit_t limit;
    offset_t offset;

    select_t() = default;
    select_t(parse_tree_node& node, table_map_t& tables);

    cell access_column(unsigned int i) override;
    void advance_row() override;
    bool empty() override;
    unsigned int width() override;
    unsigned int height() override;
    std::shared_ptr<table_iterator> load() override;

    void run() override;
};
#endif

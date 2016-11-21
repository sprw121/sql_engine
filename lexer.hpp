#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <string>

#include "table.hpp"
#include "util.hpp"

struct token_t
{
    enum token_type
    {
        PLUS,
        MINUS,
        STAR,
        DIVIDE,
        MOD,
        CARAT,
        PAREN_OPEN,
        PAREN_CLOSE,
        BANG,
        EQUAL,
        NEQUAL,
        LT,
        LTEQ,
        GT,
        GTEQ,
        AND,
        OR,
        COMMA,
        END,

        STR_LITERAL,
        INT_LITERAL,
        FLOAT_LITERAL,
        IDENTITIFER,

        SELECT,
        FROM,
        WHERE,
        LIMIT,
        OFFSET,
        AS,

        SHOW,
        TABLES,

        DESCRIBE,
        LOAD,
        EXIT,

        LEFT_JOIN,
        RIGHT_JOIN,
        OUTER_JOIN,
        INNER_JOIN,
        CROSS_JOIN,
        ON,

        FUNCTION,

        SELECT_ALL,
        NEGATE,

        INVALID
    };

    token_type  t;
    cell value;
    std::string raw_rep;

    token_t()                   : t(token_type::INVALID),       value(),   raw_rep() {};
    token_t(token_type t_)      : t(t_),                        value(),   raw_rep() {};
    token_t(token_type t_,
            std::string name_)  : t(t_),                        value(),   raw_rep(name_) {};

    token_t(long long int l_)   : t(token_type::INT_LITERAL),   value(l_), raw_rep() {};
    token_t(double d_)          : t(token_type::FLOAT_LITERAL), value(d_), raw_rep() {};
};

std::string output_token(token_t);

struct lexer
{
    std::string     query;
    unsigned int    idx;

    lexer(std::string query_) : query(query_), idx(0) {};
    bool next_token(token_t& token);

    private:
    token_t lex_operator(token_t::token_type t_);
    token_t lex_string();
    token_t lex_word();
};

#endif

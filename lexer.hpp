#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <string>

#include "boost/variant.hpp"
#include "util.hpp"

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

    LEFT,
    RIGHT,
    OUTER,
    INNER,
    CROSS,
    JOIN,
    ON,

    FUNCTION,

    INVALID
};

struct token_t
{
    token_type  t;
    boost::variant<double, long, std::string> u;

    token_t()                   : t(token_type::INVALID),       u() {};
    token_t(token_type t_)      : t(t_),                        u() {};
    token_t(token_type t_,
            std::string name_)  : t(t_),                        u(name_) {};

    token_t(long l_)            : t(token_type::INT_LITERAL),   u(l_) {};
    token_t(double d_)          : t(token_type::FLOAT_LITERAL), u(d_) {};
};

std::string output_token(token_t);

struct lexer
{
    std::string query;
    int         idx;

    lexer(std::string query_) : query(query_), idx(0) {};
    bool next_token(token_t& token);

    private:
    token_t lex_operator(token_type);
    token_t lex_string();
    token_t lex_word();
};

#endif

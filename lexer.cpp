#include <iostream>
#include <sstream>

#include "lexer.hpp"

std::string output_token(token_t t)
{
    std::stringstream stream;
    switch(t.t)
    {
        case token_type::PLUS:          stream << "PLUS"; break;
        case token_type::MINUS:         stream << "MINUS"; break;
        case token_type::STAR:          stream << "STAR"; break;
        case token_type::DIVIDE:        stream << "DIVIDE"; break;
        case token_type::MOD:           stream << "MOD"; break;
        case token_type::CARAT:         stream << "CARAT"; break;
        case token_type::PAREN_OPEN:    stream << "PAREN_OPEN"; break;
        case token_type::PAREN_CLOSE:   stream << "PAREN_CLOSE"; break;
        case token_type::BANG:          stream << "BANG"; break;
        case token_type::EQUAL:         stream << "EQUAL"; break;
        case token_type::NEQUAL:        stream << "NEQUAL"; break;
        case token_type::LT:            stream << "LT"; break;
        case token_type::LTEQ:          stream << "LTEQ"; break;
        case token_type::GT:            stream << "GT"; break;
        case token_type::GTEQ:          stream << "GTEQ";
        case token_type::COMMA:         stream << "COMMA"; break;
        case token_type::END:           stream << "END"; break;

        case token_type::STR_LITERAL:   stream << "STRING " << t.u; break;
        case token_type::INT_LITERAL:   stream << "INT " << t.u; break;
        case token_type::FLOAT_LITERAL: stream << "FLOAT " << t.u; break;
        case token_type::IDENTITIFER:   stream << "IDENTITIFER " <<t.u; break;

        case token_type::SELECT:        stream << "SELECT"; break;
        case token_type::AS:            stream << "AS"; break;
        case token_type::FROM:          stream << "FROM"; break;
        case token_type::WHERE:         stream << "WHERE"; break;
        case token_type::LIMIT:         stream << "LIMIT"; break;
        case token_type::OFFSET:        stream << "OFFSET"; break;

        case token_type::SHOW:          stream << "SHOW"; break;
        case token_type::TABLES:        stream << "TABLES"; break;

        case token_type::DESCRIBE:      stream << "DESCRIBE"; break;
        case token_type::LOAD:          stream << "LOAD"; break;
        case token_type::EXIT:          stream << "EXIT"; break;

        case token_type::LEFT:          stream << "LEFT"; break;
        case token_type::RIGHT:         stream << "RIGHT"; break;
        case token_type::OUTER:         stream << "OUTER"; break;
        case token_type::INNER:         stream << "INNER"; break;
        case token_type::CROSS:         stream << "CROSS"; break;
        case token_type::JOIN:          stream << "JOIN"; break;
        case token_type::ON:            stream << "ON"; break;

        case token_type::AND:           stream << "AND"; break;
        case token_type::OR:            stream << "OR"; break;

        case token_type::FUNCTION:      stream << "FUNCTION"; break;

        case token_type::INVALID:       stream << "INVALID"; break;
    }

    return stream.str();
}

static token_type resolve_token_string(std::string token_string)
{
    if(is_integer(token_string.c_str()))    return token_type::INT_LITERAL;
    if(is_float(token_string.c_str()))      return token_type::FLOAT_LITERAL;

    if(token_string == "select" || token_string == "SELECT")
        return token_type::SELECT;
    if(token_string == "as" || token_string == "AS")
        return token_type::AS;
    if(token_string == "from" || token_string == "FROM")
        return token_type::FROM;
    if(token_string == "where" || token_string == "WHERE")
        return token_type::WHERE;
    if(token_string == "limit" || token_string == "LIMIT")
        return token_type::LIMIT;
    if(token_string == "offset" || token_string == "OFFSET")
        return token_type::OFFSET;

    if(token_string == "show" || token_string == "SHOW")
        return token_type::SHOW;
    if(token_string == "tables" || token_string == "TABLES")
        return token_type::TABLES;

    if(token_string == "describe" || token_string == "DESCRIBE")
        return token_type::DESCRIBE;
    if(token_string == "load" || token_string == "LOAD")
        return token_type::LOAD;
    if(token_string == "exit" || token_string == "EXIT")
        return token_type::EXIT;

    if(token_string == "left" || token_string == "LEFT")
        return token_type::LEFT;
    if(token_string == "right" || token_string == "RIGHT")
        return token_type::RIGHT;
    if(token_string == "outer" || token_string == "OUTER")
        return token_type::OUTER;
    if(token_string == "inner" || token_string == "INNER")
        return token_type::INNER;
    if(token_string == "cross" || token_string == "CROSS")
        return token_type::CROSS;
    if(token_string == "join" || token_string == "JOIN")
        return token_type::JOIN;
    if(token_string == "on" || token_string == "ON")
        return token_type::ON;

    if(token_string == "and" || token_string == "AND")
        return token_type::AND;
    if(token_string == "or" || token_string == "OR")
        return token_type::OR;

    if(token_string == "f" || token_string == "f")
        return token_type::FUNCTION;

    return token_type::IDENTITIFER;
}

static token_type resolve_token_char(char input)
{
    switch(input)
    {
        case '+':
            return token_type::PLUS;
        case '-':
            return token_type::MINUS;
        case '*':
            return token_type::STAR;
        case '/':
            return token_type::DIVIDE;
        case '%':
            return token_type::MOD;
        case '^':
            return token_type::CARAT;
        case '(':
            return token_type::PAREN_OPEN;
        case ')':
            return token_type::PAREN_CLOSE;
        case '!':
            return token_type::BANG;
        case '=':
            return token_type::EQUAL;
        case '>':
            return token_type::GT;
        case '<':
            return token_type::LT;
        case ',':
            return token_type::COMMA;
        case ';':
            return token_type::END;
        case '&':
            return token_type::AND;
        case '|':
            return token_type::AND;
        default: break;
    }
    return token_type::INVALID;
}

token_t lexer::lex_operator(token_type token)
{
    idx++;
    if(idx == query.size()) return token_t(token);

    if(token == token_type::LT) // "<>"
    {
        if(resolve_token_char(query[idx]) == token_type::GT)
        {
            idx++;
            return token_t(token_type(NEQUAL));
        }
    }
    else if(token == token_type::BANG)
    {
        if(resolve_token_char(query[idx]) == token_type::EQUAL)
        {
            idx++;
            return token_t(token_type(NEQUAL));
        }
    }

    return token_t(token);
}

token_t lexer::lex_string()
{
    int start = idx;
    token_t ret;
    while(++idx < query.size() && query[idx] != query[start]){}

    if(idx == query.size())
    {
        std::cout << "Unclosed quotation: " << std::endl
                  << "     " << std::string(&query[start], idx - start) << std::endl
                  << "      ^" << std::endl;
        ret = token_t(token_type::INVALID);
    }
    else
    {
        ret = token_t(token_type::STR_LITERAL,
                        std::string(&query[start+1], idx - start - 1));
        idx++;
    }

    return ret;
}

token_t lexer::lex_word()
{
    int start = idx;
    while(++idx < query.size() &&
          resolve_token_char(query[idx]) == token_type::INVALID &&
          !is_white(query[idx]) &&
          query[idx] != '"' &&
          query[idx] != '\'');

    std::string token_string(&query[start], idx - start);
    token_type t = resolve_token_string(token_string);
    return token_t(t, std::move(token_string));
}

bool lexer::next_token(token_t& token)
{
    while(idx != query.size() && is_white(query[idx]))
    {
        idx++;
    }

    if(idx == query.size()) return false;

    token_type char_token = resolve_token_char(query[idx]);
    if(char_token != token_type::INVALID)
    {
        token = lex_operator(char_token);
    }
    else if(query[idx] == '"' || query[idx] == '\'')
    {
        token = lex_string();
    }
    else
    {
        token = lex_word();
    }

    return true;
}

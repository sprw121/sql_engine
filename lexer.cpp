#include <cstdlib>
#include <iostream>
#include <sstream>

#include "lexer.hpp"

std::string output_token(token_t t)
{
    std::stringstream stream;
    switch(t.t)
    {
        case token_t::PLUS:          stream << "PLUS";          break;
        case token_t::MINUS:         stream << "MINUS";         break;
        case token_t::STAR:          stream << "STAR";          break;
        case token_t::DIVIDE:        stream << "DIVIDE";        break;
        case token_t::MOD:           stream << "MOD";           break;
        case token_t::CARAT:         stream << "CARAT";         break;
        case token_t::PAREN_OPEN:    stream << "PAREN_OPEN";    break;
        case token_t::PAREN_CLOSE:   stream << "PAREN_CLOSE";   break;
        case token_t::BANG:          stream << "BANG";          break;
        case token_t::EQUAL:         stream << "EQUAL";         break;
        case token_t::NEQUAL:        stream << "NEQUAL";        break;
        case token_t::LT:            stream << "LT";            break;
        case token_t::LTEQ:          stream << "LTEQ";          break;
        case token_t::GT:            stream << "GT";            break;
        case token_t::GTEQ:          stream << "GTEQ";          break;
        case token_t::COMMA:         stream << "COMMA";         break;
        case token_t::END:           stream << "END";           break;

        case token_t::STR_LITERAL:   stream << "STRING "      << t.value; break;
        case token_t::INT_LITERAL:   stream << "INT "         << t.value; break;
        case token_t::FLOAT_LITERAL: stream << "FLOAT "       << t.value; break;
        case token_t::IDENTITIFER:   stream << "IDENTITIFER " << t.value; break;

        case token_t::SELECT:        stream << "SELECT";    break;
        case token_t::AS:            stream << "AS";        break;
        case token_t::FROM:          stream << "FROM";      break;
        case token_t::WHERE:         stream << "WHERE";     break;
        case token_t::LIMIT:         stream << "LIMIT";     break;
        case token_t::OFFSET:        stream << "OFFSET";    break;

        case token_t::SHOW:          stream << "SHOW";      break;
        case token_t::TABLES:        stream << "TABLES";    break;

        case token_t::DESCRIBE:      stream << "DESCRIBE";  break;
        case token_t::LOAD:          stream << "LOAD";      break;
        case token_t::EXIT:          stream << "EXIT";      break;

        case token_t::LEFT_JOIN:     stream << "LEFT";      break;
        case token_t::RIGHT_JOIN:    stream << "RIGHT";     break;
        case token_t::OUTER_JOIN:    stream << "OUTER";     break;
        case token_t::INNER_JOIN:    stream << "INNER";     break;
        case token_t::CROSS_JOIN:    stream << "CROSS";     break;
        case token_t::ON:            stream << "ON";        break;

        case token_t::AND:           stream << "AND";       break;
        case token_t::OR:            stream << "OR";        break;

        case token_t::FUNCTION:      stream << "FUNCTION";  break;

        case token_t::SELECT_ALL:    stream << "SELECT_ALL";    break;
        case token_t::NEGATE:        stream << "NEGATE";        break;
        case token_t::INVALID:       stream << "INVALID";       break;
    }

    return stream.str();
}

static token_t::token_type resolve_token_string(std::string token_string)
{
    if(is_integer(token_string.c_str()))    return token_t::INT_LITERAL;
    if(is_float(token_string.c_str()))      return token_t::FLOAT_LITERAL;

    if(token_string == "select" || token_string == "SELECT")
        return token_t::SELECT;
    if(token_string == "as" || token_string == "AS")
        return token_t::AS;
    if(token_string == "from" || token_string == "FROM")
        return token_t::FROM;
    if(token_string == "where" || token_string == "WHERE")
        return token_t::WHERE;
    if(token_string == "limit" || token_string == "LIMIT")
        return token_t::LIMIT;
    if(token_string == "offset" || token_string == "OFFSET")
        return token_t::OFFSET;

    if(token_string == "show" || token_string == "SHOW")
        return token_t::SHOW;
    if(token_string == "tables" || token_string == "TABLES")
        return token_t::TABLES;

    if(token_string == "describe" || token_string == "DESCRIBE")
        return token_t::DESCRIBE;
    if(token_string == "load" || token_string == "LOAD")
        return token_t::LOAD;
    if(token_string == "exit" || token_string == "EXIT")
        return token_t::EXIT;

    if(token_string == "left_join" || token_string == "LEFT_JOIN" ||
       token_string == "left_outer_join" || token_string == "LEFT_OUTER_JOIN")
        return token_t::LEFT_JOIN;
    if(token_string == "right_join" || token_string == "RIGHT_JOIN" ||
       token_string == "right_outer_join" || token_string == "RIGHT_OUTER_JOIN")
        return token_t::RIGHT_JOIN;
    if(token_string == "outer_join" || token_string == "OUTER_JOIN")
        return token_t::OUTER_JOIN;
    if(token_string == "inner_join" || token_string == "INNER_JOIN")
        return token_t::INNER_JOIN;
    if(token_string == "cross_join" || token_string == "CROSS_JOIN")
        return token_t::CROSS_JOIN;
    if(token_string == "on" || token_string == "ON")
        return token_t::ON;

    if(token_string == "and" || token_string == "AND")
        return token_t::AND;
    if(token_string == "or" || token_string == "OR")
        return token_t::OR;

    if(token_string == "f" || token_string == "f")
        return token_t::FUNCTION;

    return token_t::IDENTITIFER;
}

static token_t::token_type resolve_token_char(char input)
{
    switch(input)
    {
        case '+':
            return token_t::PLUS;
        case '-':
            return token_t::MINUS;
        case '*':
            return token_t::STAR;
        case '/':
            return token_t::DIVIDE;
        case '%':
            return token_t::MOD;
        case '^':
            return token_t::CARAT;
        case '(':
            return token_t::PAREN_OPEN;
        case ')':
            return token_t::PAREN_CLOSE;
        case '!':
            return token_t::BANG;
        case '=':
            return token_t::EQUAL;
        case '>':
            return token_t::GT;
        case '<':
            return token_t::LT;
        case ',':
            return token_t::COMMA;
        case ';':
            return token_t::END;
        case '&':
            return token_t::AND;
        case '|':
            return token_t::AND;
        default: break;
    }
    return token_t::INVALID;
}

token_t lexer::lex_operator(token_t::token_type token)
{
    idx++;
    if(idx == query.size()) return token_t(token);

    if(token == token_t::LT) // <>, <=
    {
        if(resolve_token_char(query[idx]) == token_t::GT)
        {
            idx++;
            return token_t(token_t::NEQUAL);
        }
        else if(resolve_token_char(query[idx]) == token_t::EQUALS)
        {
            idx++;
            return token_t(token_t::NEQUAL);
        }
    }
    if(token == token_t::GT) // >=
    {
        if(resolve_token_char(query[idx]) == token_t::EQUALS)
        {
            idx++;
            return token_t(token_t::GTEQ);
        }
    }
    else if(token == token_t::BANG)
    {
        if(resolve_token_char(query[idx]) == token_t::EQUAL)
        {
            idx++;
            return token_t(token_t::NEQUAL);
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
        ret = token_t(token_t::INVALID);
    }
    else
    {
        ret = token_t(token_t::STR_LITERAL,
                        std::string(&query[start+1], idx - start - 1));
        idx++;
    }

    return ret;
}

token_t lexer::lex_word()
{
    int start = idx;
    while(++idx < query.size() &&
          resolve_token_char(query[idx]) == token_t::INVALID &&
          !is_white(query[idx]) &&
          query[idx] != '"' &&
          query[idx] != '\'');

    std::string token_string(&query[start], idx - start);
    token_t::token_type t = resolve_token_string(token_string);
    if(t == token_t::INT_LITERAL)
        return token_t(atoll(token_string.c_str()));
    if(t == token_t::FLOAT_LITERAL)
        return token_t(atof(token_string.c_str()));
    return token_t(t, token_string);
}

bool lexer::next_token(token_t& token)
{
    while(idx != query.size() && is_white(query[idx]))
    {
        idx++;
    }

    if(idx == query.size()) return false;

    token_t::token_type char_token = resolve_token_char(query[idx]);
    if(char_token != token_t::INVALID)
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

#ifndef PARSER_H
#define PARSER_H

#include "Expression.h"
#include <string>

class Parser {
public:
    Expression parseExpression(std::vector<std::string> &tokens, size_t &currentIndex);
    Expression parseWhereClause(const std::string& whereClause);

};

#endif // PARSER_H


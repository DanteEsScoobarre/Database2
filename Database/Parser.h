#ifndef PARSER_H
#define PARSER_H

#include "Expression.h"
#include <string>

class Parser {
public:
    auto isComparisonOperator(const std::string &token)-> bool;
    auto parseExpression(std::vector<std::string> &tokens, size_t &currentIndex) -> std::unique_ptr<Expression>;
    std::unique_ptr<Expression> parseWhereClause(const std::string& whereClause);


};

#endif // PARSER_H


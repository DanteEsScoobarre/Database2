#include "Parser.h"


auto tokenize(const std::string &str, char delimiter) -> std::vector<std::string> {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}



Expression Parser::parseExpression(std::vector<std::string> &tokens, size_t &currentIndex) {
    Expression expr;
    std::unique_ptr<Expression> current = std::make_unique<Expression>();

    while (currentIndex < tokens.size()) {
        const auto &token = tokens[currentIndex++];

        if (token == "(") {
            *current = parseExpression(tokens, currentIndex);
        } else if (token == ")") {
            break;
        } else if (token == "and" || token == "or") {
            // Logical operators
            Expression newExpr;
            newExpr.logicalOperator = token;
            newExpr.left = std::move(current);
            current = std::make_unique<Expression>();
            newExpr.right = std::move(current);
            expr = std::move(newExpr);
        } else if (token == "=" || token == ">" || token == "<" || token == ">=" || token == "<=" || token == "!=") {

            current->operators = token;
        } else {
            if (current->column.empty()) {
                current->column = token;
            } else {
                current->value = token;
            }
        }
    }

    return expr;
}
Expression Parser::parseWhereClause(const std::string &whereClause) {
    auto tokens = tokenize(whereClause, ' ');
    size_t currentIndex = 0;
    return parseExpression(tokens, currentIndex);
}

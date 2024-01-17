#include "Parser.h"


auto tokenize(const std::string &str, char delimiter) -> std::vector<std::string> {
    std::vector<std::string> tokens;
    std::string currentToken;

    for (char ch: str) {
        if (std::isspace(ch)) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else if (std::ispunct(ch)) {
            // Add the current token if it's not empty
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            // Add the punctuation (operator/parentheses) as a separate token
            tokens.push_back(std::string(1, ch));
        } else {
            currentToken += ch;
        }
    }

    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }

    return tokens;
}


std::unique_ptr<Expression> Parser::parseWhereClause(const std::string &whereClause) {
    auto tokens = tokenize(whereClause, ' ');
    size_t currentIndex = 0;
    return parseExpression(tokens, currentIndex);
}

auto Parser::isComparisonOperator(const std::string &token) -> bool {
    return token == "=" || token == "!=" || token == ">" || token == "<" || token == ">=" || token == "<=";
}

auto Parser::parseExpression(std::vector<std::string> &tokens, size_t &currentIndex) -> std::unique_ptr<Expression> {
    auto expr = std::make_unique<Expression>();

    while (currentIndex < tokens.size()) {
        const auto &token = tokens[currentIndex];

        if (token == "(") {
            ++currentIndex; // Skip the opening parenthesis
            expr = parseExpression(tokens, currentIndex); // Correctly assign the unique_ptr
            if (currentIndex >= tokens.size() || tokens[currentIndex] != ")") {
                throw std::runtime_error("Mismatched parentheses in expression");
            }
            ++currentIndex; // Skip the closing parenthesis
        } else if (token == "AND" || token == "OR") {
            // Handle logical operators
            auto newExpr = std::make_unique<Expression>();
            newExpr->logicalOperator = token;
            newExpr->left = std::move(expr); // Move the current expression to left
            ++currentIndex; // Move to the next part of the expression
            newExpr->right = parseExpression(tokens, currentIndex); // Assign unique_ptr directly
            expr = std::move(newExpr);
        } else if (isComparisonOperator(token)) {
            // Handle comparison operators
            if (expr->column.empty() || !expr->value.empty()) {
                throw std::runtime_error("Invalid expression format");
            }
            expr->operators = token;
            ++currentIndex; // Move to the next token, which should be the value
            if (currentIndex < tokens.size()) {
                expr->value = tokens[currentIndex];
                ++currentIndex;
            } else {
                throw std::runtime_error("Expected value after operator");
            }
        } else {
            // This should be a column name
            if (!expr->column.empty()) {
                throw std::runtime_error("Unexpected token: " + token);
            }
            expr->column = token;
            ++currentIndex;
        }

        if (token == "AND" || token == "OR") {
            break;
        }
    }

    return expr;
}



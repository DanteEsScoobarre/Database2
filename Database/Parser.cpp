#include "Parser.h"

auto Parser::tokenize(const std::string &str, char delimiter) -> std::vector<std::string> {
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

auto Parser::parseSQLCommand(const std::string &commandStr) -> Command {
    std::vector<std::string> tokens = tokenize(commandStr, ' ');
    Command cmd;

    if (tokens.empty()) {
        throw std::runtime_error("Empty command string");
    }

    cmd.type = tokens[0];
    if (cmd.type == "CREATE") {
        parseCreateCommand(tokens, cmd);
    } else if (cmd.type == "DROP") {
        parseDropCommand(tokens, cmd);
    } else if (cmd.type == "ADD") {
        parseAddCommand(tokens, cmd);
    } else if (cmd.type == "SELECT") {
        parseSelectCommand(tokens, cmd);
    } else if (cmd.type == "UPDATE") {
        parseUpdateCommand(tokens, cmd);
    } else if (cmd.type == "DELETE") {
        parseDeleteDataCommand(tokens, cmd);
    } else if (cmd.type == "INSERT") {
        parseInsertCommand(tokens, cmd);
    } else if (cmd.type == "REMOVE") {
        parseDeleteColumnCommand(tokens, cmd);
    }


    return cmd;
}






auto Parser::parseCreateCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() < 3 || tokens[1] != "table_name" || tokens[2] != "WITH") {
        throw std::runtime_error("Invalid syntax for CREATE command");
    }
    cmd.tableName = tokens[1];
    size_t i = 3;
    while (i < tokens.size()) {
        if (tokens[i] != "{") {
            throw std::runtime_error("Expected '{' before column definition");
        }

        if (i + 3 >= tokens.size() || tokens[i + 3] != "}") {
            throw std::runtime_error("Expected '}' after column definition");
        }

        Column column;
        column.name = tokens[i + 1];
        column.type = tokens[i + 2];

        cmd.columns.push_back(column);
        i += 4;
    }
}


auto Parser::parseDropCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() != 2) {
        throw std::runtime_error("Invalid syntax for DROP command");
    }

    cmd.tableName = tokens[1];
}

auto Parser::parseAddCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() < 6 || tokens[1] != "{" || tokens[3] != "}" || tokens[4] != "INTO") {
        throw std::runtime_error("Invalid syntax for ADD command");
    }

    ColumnDefinition columnDef;
    columnDef.name = tokens[2]; // Assuming the third token is the column name
    columnDef.type = tokens[3]; // Assuming the fourth token is the column type

    Column column = convertToColumn(columnDef);
    cmd.columns.push_back(column);
    cmd.tableName = tokens[5]; // Assuming the sixth token is the table name
}

auto Parser::parseSelectCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() < 4 || tokens[2] != "FROM") {
        throw std::runtime_error("Invalid syntax for SELECT command");
    }

    cmd.type = "SELECT";
    size_t i = 1; // Start from the token after "SELECT"

    // Parse column names until "FROM"
    while (tokens[i] != "FROM") {
        cmd.columns.push_back({tokens[i], ""}); // Only name is needed, type is not relevant for SELECT
        i++;
        if (i >= tokens.size()) {
            throw std::runtime_error("Missing 'FROM' keyword in SELECT command");
        }
        if (tokens[i] == ",") i++;
    }

    cmd.tableName = tokens[++i];

    // Check for WHERE clause
    if (i + 1 < tokens.size() && tokens[i + 1] == "WHERE") {
        for (i += 2; i < tokens.size(); ++i) {
            if (!cmd.whereClause.empty()) {
                cmd.whereClause += " ";
            }
            cmd.whereClause += tokens[i];
        }
    }
}



auto Parser::parseUpdateCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() < 6 || tokens[2] != "FROM" || tokens[4] != "WITH" || tokens[5] != "[") {
        throw std::runtime_error("Invalid syntax for UPDATE command");
    }

    cmd.type = "UPDATE";
    cmd.columnName = tokens[1]; // Column name to be updated
    cmd.tableName = tokens[3]; // Table name

    // Initialize an empty Row object for updatedData
    cmd.updatedData = Row();

    // Extracting the updated data and its type
    size_t i = 6; // Start from the token after '['
    while (i < tokens.size() && tokens[i] != "]") {
        cmd.updatedData.Data.push_back(tokens[i]); // Add data to the Row's Data vector
        i++;
    }

    if (i >= tokens.size() || tokens[i] != "]") {
        throw std::runtime_error("Expected ']' in UPDATE command");
    }
}

auto Parser::parseDeleteColumnCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() != 4 || tokens[2] != "FROM") {
        throw std::runtime_error("Invalid syntax for REMOVE command");
    }

    cmd.type = "REMOVE";
    cmd.columnName = tokens[1]; // The column name to be removed
    cmd.tableName = tokens[3]; // The table name
}


auto Parser::parseInsertCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() < 6 || tokens[1] != "[" || tokens[3] != "INTO" || tokens[5] != "IN") {
        throw std::runtime_error("Invalid syntax for INSERT command");
    }

    cmd.type = "INSERT";
    cmd.columnName = tokens[4]; // The column name
    cmd.tableName = tokens[6]; // The table name

    // Initialize an empty Row object
    cmd.data = Row();

    // Handle the data to be inserted
    // Assuming the first data item is inside brackets [data]
    cmd.data.Data.push_back(tokens[2]); // Add the first data item

    // Handle any additional data if provided in the command
    for (size_t i = 7; i < tokens.size(); ++i) {
        cmd.data.Data.push_back(tokens[i]);
    }
}

void Parser::parseDeleteDataCommand(std::vector<std::string> &tokens, Command &cmd) {
    if (tokens.size() != 5 || tokens[1] != "FROM" || tokens[3] != "IN") {
        throw std::runtime_error("Invalid syntax for DELETE command");
    }

    cmd.type = "DELETE";
    cmd.columnName = tokens[2]; // The column name from which data will be deleted
    cmd.tableName = tokens[4]; // The table name
}

Column Parser::convertToColumn(const ColumnDefinition& columnDef) {
    Column column;
    column.name = columnDef.name;
    column.type = columnDef.type;
    return column;
}

auto Parser::parseWhereClause(const std::string &whereClause) -> std::unique_ptr<Expression> {
    if (whereClause.empty()) {
        return nullptr; // No condition provided
    }

    std::vector<std::string> tokens = tokenize(whereClause, ' ');
    size_t currentIndex = 0;
    return parseExpression(tokens, currentIndex);
}





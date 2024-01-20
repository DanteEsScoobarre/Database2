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
const std::vector<Column> Command::emptyColumns = {};
auto Parser::parseSQLCommand(const std::string &commandStr) -> Command {
    std::vector<std::string> tokens = tokenize(commandStr, ' ');
    if (tokens.empty()) {
        throw std::runtime_error("Empty command string");
    }

    Command cmd;

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
    } else if (cmd.type == "SAVE"){
        parseSaveCommand(tokens, cmd);
    } else if (cmd.type == "LOAD") {
        parseLoadCommand(tokens, cmd);
    }
    else {
        throw std::runtime_error("Unknown command type: " + cmd.type);
    }

    return cmd;
}




auto Parser::parseCreateCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() < 7 || tokens[2] != "WITH") {
        throw std::runtime_error("Invalid syntax for CREATE command");
    }

    cmd.type = "CREATE";
    cmd.tableName = tokens[1];

    size_t i = 3; // Start from the token after "WITH"
    while (i + 4 < tokens.size()) {
        if (tokens[i] != "{" || tokens[i + 4] != "}") {
            throw std::runtime_error("Malformed column definition at position " + std::to_string(i));
        }

        if (tokens[i + 2] != ",") {
            throw std::runtime_error("Expected ',' after column name at position " + std::to_string(i + 2));
        }

        Column column;
        column.name = tokens[i + 1]; // Column name
        column.type = tokens[i + 3]; // Column data type

        cmd.columns.push_back(column);
        i += (i + 6 < tokens.size() && tokens[i + 6] == "{") ? 6 : 5; // Move to the next column definition
    }
}

auto Parser::parseDropCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() != 2) {
        throw std::runtime_error("Invalid syntax for DROP command");
    }

    cmd.tableName = tokens[1];
}

auto Parser::parseAddCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    auto startBracketPos = std::ranges::find(tokens.begin(), tokens.end(), "{");
    auto endBracketPos = std::ranges::find(tokens.begin(), tokens.end(), "}");

    if (startBracketPos == tokens.end() || endBracketPos == tokens.end() || endBracketPos <= startBracketPos) {
        throw std::runtime_error("Invalid syntax for ADD command: Brackets not found or malformed");
    }

    if ((endBracketPos + 2 >= tokens.end()) || *(endBracketPos + 1) != "INTO") {
        throw std::runtime_error("Invalid syntax for ADD command: 'INTO' not found or misplaced");
    }

    std::string columnName = *(startBracketPos + 1);
    std::string columnType = *(startBracketPos + 2);

    if (*(startBracketPos + 2) != ",") {
        throw std::runtime_error("Invalid syntax for ADD command: Missing comma in column definition");
    }

    Column column = {columnName, columnType};
    cmd.columns.push_back(column);
    cmd.tableName = *(endBracketPos + 2); // Table name is after "INTO"
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

    // Extracting the updated data
    size_t i = 6; // Start from the token after '['
    if (i < tokens.size() && tokens[i] != "]") {
        std::string updatedValue = tokens[i]; // Extract the new value

        // Initialize a Row object with an empty columns vector for now
        cmd.updatedData = Row(std::vector<Column>());
        cmd.updatedData.Data.push_back(updatedValue); // Add the new value to the Row's Data vector
    } else {
        throw std::runtime_error("Expected new value in UPDATE command");
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
    auto intoPos = std::ranges::find(tokens.begin(), tokens.end(), "INTO");
    if (intoPos == tokens.end() || std::distance(tokens.begin(), intoPos) < 3 || *(intoPos + 2) != "IN") {
        throw std::runtime_error("Invalid syntax for INSERT command");
    }

    cmd.type = "INSERT";
    cmd.columnName = *(intoPos + 1); // The column name
    cmd.tableName = *(intoPos + 3); // The table name

    // Reconstruct the data string from tokens[1] to the token before "INTO"
    std::string data;
    for (auto it = tokens.begin() + 1; it != intoPos; ++it) {
        data += (it != tokens.begin() + 1 ? " " : "") + *it;
    }
    // Remove the surrounding brackets from the data
    if (data.front() == '[' && data.back() == ']') {
        data = data.substr(1, data.length() - 2);
    }

    // Initialize an empty Row object and add the data
    cmd.data = Row(std::vector<Column>());
    cmd.data.Data.push_back(data);
}


void Parser::parseDeleteDataCommand(std::vector<std::string> &tokens, Command &cmd) {  // Find the position of "FROM" and "IN" in the tokens
    auto fromPos = std::find(tokens.begin(), tokens.end(), "FROM");
    auto inPos = std::find(tokens.begin(), tokens.end(), "IN");

    if (fromPos == tokens.end() || inPos == tokens.end() || fromPos > inPos || tokens.begin() + 1 >= fromPos) {
        throw std::runtime_error("Invalid syntax for DELETE command");
    }

    cmd.type = "DELETE";
    cmd.tableName = *(inPos + 1); // Table name is the token after "IN"

    // Reconstruct the data string from tokens[1] to the token before "FROM"
    std::string data;
    for (auto it = tokens.begin() + 1; it != fromPos; ++it) {
        data += (it != tokens.begin() + 1 ? " " : "") + *it;
    }
    // Remove the surrounding brackets from the data
    if (data.front() == '[' && data.back() == ']') {
        data = data.substr(1, data.length() - 2);
    }

    cmd.columnName = *(fromPos + 1); // Column name is the token after "FROM"
    cmd.whereClause = data; // Use the reconstructed data as the whereClause
}




auto Parser::parseWhereClause(const std::string &whereClause) -> std::unique_ptr<Expression> {
    if (whereClause.empty()) {
        return nullptr; // No condition provided
    }

    std::vector<std::string> tokens = tokenize(whereClause, ' ');
    size_t currentIndex = 0;
    return parseExpression(tokens, currentIndex);
}

auto
Parser::extractAndValidateData(const std::string &dataStr, const std::string &dataType) -> std::vector<std::string> {
    std::vector<std::string> dataTokens = tokenize(dataStr, ',');
    std::vector<std::string> validatedData;
    std::vector<std::string> invalidData;

    for (const auto& token : dataTokens) {
        if (validateDataType(token, dataType)) {
            validatedData.push_back(token);
        } else {
            invalidData.push_back(token);
        }
    }

    if (!invalidData.empty()) {
        std::string errorMessage = "Invalid data detected: ";
        for (const auto& invalid : invalidData) {
            errorMessage += invalid + " ";
        }
        throw std::runtime_error(errorMessage);
    }

    return validatedData;
}

auto Parser::joinValidatedData(const std::vector<std::string>& data) -> std::string {
    std::string result;
    for (const auto& item : data) {
        if (!result.empty()) {
            result += ", ";
        }
        result += item;
    }
    return result;
}

auto Parser::validateDataType(const std::string &value, const std::string &type) -> bool {
    if (type == "int") {
        return isInteger(value);
    } else if (type == "bool") {
        return isBoolean(value);
    } else if (type == "string") {
        return true;
    } else {
        // Handle other types or throw an error
    }
    return false;
}
auto Parser::isBoolean(const std::string &value) -> bool {
    return (value == "true" || value == "false" || value == "NULL");
}

auto Parser::isInteger(const std::string &value) -> bool {
    if (value.empty() || ((!isdigit(value[0])) && (value[0] != '-') && (value[0] != '+'))) return false;

    char * p;
    strtol(value.c_str(), &p, 10);

    return (*p == 0);
}

auto Parser::parseSaveCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() < 2) {
        throw std::runtime_error("Invalid syntax for SAVE command");
    }

    cmd.type = "SAVE";
    std::vector<std::string> filePathTokens(tokens.begin() + 1, tokens.end());
    cmd.value = joinFilePath(filePathTokens); // 'value' field will store the file path
}

auto Parser::parseLoadCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
        if (tokens.size() < 2) {
            throw std::runtime_error("Invalid syntax for LOAD command");
        }

        cmd.type = "LOAD";
        // Since the file path might contain spaces, we join all the tokens back into a single string
        std::string filePath = joinFilePath(std::vector<std::string>(tokens.begin() + 1, tokens.end()));
        cmd.value = filePath;
    }

auto Parser::joinFilePath(const std::vector<std::string>& pathTokens) -> std::string {
    std::string filePath;
    for (const auto& token : pathTokens) {
        if (!filePath.empty()) {
            filePath += "";
        }
        filePath += token;
    }
    return filePath;
}



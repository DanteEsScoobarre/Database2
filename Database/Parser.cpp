#include "Parser.h"
/*
Tokenizacja danych, parsowanie po nich.
 https://kishoreganesh.com/post/writing-a-json-parser-in-cplusplus/

*/
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

            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }

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
    if (currentIndex >= tokens.size()) {
        throw std::runtime_error("Expression parsing reached unexpected end of tokens");
    }

    auto expr = std::make_unique<Expression>();

    while (currentIndex < tokens.size()) {
        const auto& token = tokens[currentIndex];

        if (token == "(") {
            ++currentIndex;
            expr = parseExpression(tokens, currentIndex);
            if (currentIndex >= tokens.size() || tokens[currentIndex] != ")") {
                throw std::runtime_error("Mismatched parentheses in expression");
            }
            ++currentIndex;
        } else if (isComparisonOperator(token)) {

            expr->operators = token;
            ++currentIndex;
            if (currentIndex < tokens.size()) {
                expr->value = tokens[currentIndex];
                ++currentIndex;
            } else {
                throw std::runtime_error("Expected value after operator");
            }
        } else if (isLogicalOperator(token)) {
            auto newExpr = std::make_unique<Expression>();
            newExpr->logicalOperator = token;
            newExpr->left = std::move(expr);
            ++currentIndex;
            newExpr->right = parseExpression(tokens, currentIndex);
            expr = std::move(newExpr);
            if (currentIndex < tokens.size() && isComparisonOperator(tokens[currentIndex])) {
                continue;
            }
            break;
        } else {

            expr->column = token;
            ++currentIndex;
        }
    }

    return expr;
}

bool Parser::isLogicalOperator(const std::string &token) {
    return token == "AND" || token == "OR";
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

    size_t i = 3;
    while (i + 4 < tokens.size()) {
        if (tokens[i] != "{" || tokens[i + 4] != "}") {
            throw std::runtime_error("Malformed column definition at position " + std::to_string(i));
        }

        if (tokens[i + 2] != ",") {
            throw std::runtime_error("Expected ',' after column name at position " + std::to_string(i + 2));
        }

        Column column;
        column.name = tokens[i + 1];
        column.type = tokens[i + 3];

        cmd.columns.push_back(column);
        i += (i + 6 < tokens.size() && tokens[i + 6] == "{") ? 6 : 5;
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
    std::string columnType = *(endBracketPos - 1);

    if (*(startBracketPos + 2) != ",") {
        throw std::runtime_error("Invalid syntax for ADD command: Missing comma in column definition");
    }

    Column column = {columnName, columnType};
    cmd.columns.push_back(column);
    cmd.tableName = *(endBracketPos + 2);
}

auto Parser::parseSelectCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() < 4 || tokens[2] != "FROM") {
        throw std::runtime_error("Invalid syntax for SELECT command");
    }

    cmd.type = "SELECT";
    size_t i = 1;


    while (tokens[i] != "FROM") {
        cmd.columns.push_back({tokens[i], ""});
        i++;
        if (i >= tokens.size()) {
            throw std::runtime_error("Missing 'FROM' keyword in SELECT command");
        }
        if (tokens[i] == ",") i++;
    }

    cmd.tableName = tokens[++i];

    if (i + 1 < tokens.size() && tokens[i + 1] == "WHERE") {
        std::string whereClause;
        for (i += 2; i < tokens.size(); ++i) {
            if (!whereClause.empty()) {
                whereClause += " ";
            }
            whereClause += tokens[i];
        }

        cmd.whereExpression = parseWhereClause(whereClause);
    }
}

auto Parser::parseUpdateCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() < 6 || tokens[2] != "FROM" || tokens[4] != "WITH") {
        throw std::runtime_error("Invalid syntax for UPDATE command");
    }

    cmd.type = "UPDATE";
    cmd.columnName = tokens[1];
    cmd.tableName = tokens[3];


    size_t valueStartIndex = 5;
    if (tokens[valueStartIndex] != "[") {
        throw std::runtime_error("Expected '[' in UPDATE command");
    }


    if (valueStartIndex + 1 < tokens.size() && tokens[valueStartIndex + 1] != "]") {
        std::string updatedValue = tokens[valueStartIndex + 1];


        cmd.updatedData = Row(std::vector<Column>());
        cmd.updatedData.Data.push_back(updatedValue);
    } else {
        throw std::runtime_error("Expected new value in UPDATE command");
    }


    size_t whereIndex = valueStartIndex + 2;
    if (whereIndex < tokens.size() && tokens[whereIndex] == "WHERE") {

        std::string whereClause;
        for (size_t i = whereIndex + 1; i < tokens.size(); ++i) {
            if (!whereClause.empty()) {
                whereClause += " ";
            }
            whereClause += tokens[i];
        }
        cmd.whereClause = whereClause;
    }
}

auto Parser::parseDeleteColumnCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() != 4 || tokens[2] != "FROM") {
        throw std::runtime_error("Invalid syntax for REMOVE command");
    }

    cmd.type = "REMOVE";
    cmd.columnName = tokens[1];
    cmd.tableName = tokens[3];
}


auto Parser::parseInsertCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    auto intoPos = std::ranges::find(tokens.begin(), tokens.end(), "INTO");
    if (intoPos == tokens.end() || std::distance(tokens.begin(), intoPos) < 3 || *(intoPos + 2) != "IN") {
        throw std::runtime_error("Invalid syntax for INSERT command");
    }

    cmd.type = "INSERT";
    cmd.columnName = *(intoPos + 1);
    cmd.tableName = *(intoPos + 3);


    std::string data;
    for (auto it = tokens.begin() + 1; it != intoPos; ++it) {
        data += (it != tokens.begin() + 1 ? " " : "") + *it;
    }


    if (data.front() == '[' && data.back() == ']') {
        data = data.substr(1, data.length() - 2);
        data = trim(data);
    } else {
        throw std::runtime_error("Invalid data format: " + data);
    }

   \
    if (data.front() == '\'' && data.back() == '\'') {
        data = data.substr(1, data.length() - 2);
    } else if (data.front() == '(' && data.back() == ')') {
        data = data.substr(1, data.length() - 2);
    } else if (std::all_of(data.begin(), data.end(), ::isdigit) || (data.front() == '-' && std::all_of(data.begin() + 1, data.end(), ::isdigit))) {

    } else {
        throw std::runtime_error("Unrecognized data format: " + data);
    }

    cmd.data = Row(std::vector<Column>());
    cmd.data.Data.push_back(data);
}

std::string Parser::trim(const std::string &str) {
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

auto Parser::parseDeleteDataCommand(std::vector<std::string> &tokens, Command &cmd) -> void {
    auto fromPos = std::find(tokens.begin(), tokens.end(), "FROM");
    auto inPos = std::find(tokens.begin(), tokens.end(), "IN");

    if (fromPos == tokens.end() || inPos == tokens.end() || fromPos > inPos || tokens.begin() + 1 >= fromPos) {
        throw std::runtime_error("Invalid syntax for DELETE command");
    }

    cmd.type = "DELETE";
    cmd.tableName = *(inPos + 1);


    std::string dataToDelete;
    for (auto it = tokens.begin() + 1; it != fromPos; ++it) {
        dataToDelete += (it != tokens.begin() + 1 ? " " : "") + *it;
    }

    if (dataToDelete.front() == '[' && dataToDelete.back() == ']') {
        dataToDelete = dataToDelete.substr(1, dataToDelete.length() - 2);
        dataToDelete = trim(dataToDelete);
    } else {
        throw std::runtime_error("Invalid data format: " + dataToDelete);
    }

        if (dataToDelete.front() == '\'' && dataToDelete.back() == '\'') {
        dataToDelete = dataToDelete.substr(1, dataToDelete.length() - 2);
    } else if (dataToDelete.front() == '(' && dataToDelete.back() == ')') {
        dataToDelete = dataToDelete.substr(1, dataToDelete.length() - 2);
    } else if (std::all_of(dataToDelete.begin(), dataToDelete.end(), ::isdigit) || (dataToDelete.front() == '-' && std::all_of(dataToDelete.begin() + 1, dataToDelete.end(), ::isdigit))) {
        // Integer data - no additional processing needed
    } else {
        throw std::runtime_error("Unrecognized data format: " + dataToDelete);
    }

    cmd.columnName = *(fromPos + 1);
    cmd.dataToDelete = dataToDelete;
}



auto Parser::parseWhereClause(const std::string &whereClause) -> std::unique_ptr<Expression> {
    if (whereClause.empty()) {
        return nullptr;
    }

    std::vector<std::string> tokens = tokenize(whereClause, ' ');
    size_t currentIndex = 0;
    return parseExpression(tokens, currentIndex);
}



auto Parser::parseSaveCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
    if (tokens.size() < 2) {
        throw std::runtime_error("Invalid syntax for SAVE command");
    }

    cmd.type = "SAVE";
    std::vector<std::string> filePathTokens(tokens.begin() + 1, tokens.end());
    cmd.value = joinFilePath(filePathTokens);
}

auto Parser::parseLoadCommand(const std::vector<std::string> &tokens, Command &cmd) -> void {
        if (tokens.size() < 2) {
            throw std::runtime_error("Invalid syntax for LOAD command");
        }

        cmd.type = "LOAD";
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












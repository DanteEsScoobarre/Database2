#ifndef PARSER_H
#define PARSER_H
#pragma once
#include "Expression.h"
#include "Database.h"
#include "Row.h"
#include "Column.h"


struct ColumnDefinition {
    std::string name;
    std::string type;
};
struct Command {

    std::string type; // CREATE, DROP, ADD, etc.
    std::string tableName;
    std::vector<Column> columns;
    std::string whereClause;
    std::string columnName;
    Row updatedData;
    Row data;
    std::vector<std::string> additionalData;
};

class Parser {
public:
   auto tokenize(const std::string &str, char delimiter) -> std::vector<std::string>;
    auto parseSQLCommand(const std::string &commandStr) -> Command;
    auto parseWhereClause(const std::string &whereClause)-> std::unique_ptr<Expression>;
    auto parseExpression(std::vector<std::string> &tokens, size_t &currentIndex) -> std::unique_ptr<Expression>;
    auto isComparisonOperator(const std::string &token) -> bool;
private:

    auto parseCreateCommand(const std::vector<std::string> &tokens, Command &cmd) -> void;
    auto parseDropCommand(const std::vector<std::string> &tokens, Command &cmd) -> void;
    auto parseAddCommand(const std::vector<std::string> &tokens, Command &cmd) -> void;
    auto parseSelectCommand(const std::vector<std::string> &tokens, Command &cmd) -> void;
    auto parseUpdateCommand(const std::vector<std::string> &tokens, Command &cmd) -> void;
    auto parseDeleteColumnCommand(const std::vector<std::string> &tokens, Command &cmd) -> void;
    auto parseInsertCommand(const std::vector<std::string> &tokens, Command &cmd) -> void;
    auto parseDeleteDataCommand(std::vector<std::string> &token, Command &cmd) -> void;
    auto convertToColumn(const ColumnDefinition& columnDef) -> Column;





};

#endif // PARSER_H


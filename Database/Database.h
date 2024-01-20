#ifndef DATABASE2_DATABASE_H
#define DATABASE2_DATABASE_H
#pragma once

#include "Prerequestion.h"
#include "Expression.h"
#include "Parser.h"
#include "Column.h"
#include "Row.h"
#include "Table.h"

class Database {
public:
     Database() = default;

    auto isBoolean(const std::string &value) -> bool;
    auto isInteger(const std::string &value) -> bool;
    bool isString(const std::string &value);

    // DDL Operations
    auto createTable(const std::string &tableName, const std::vector<Column> &columns) -> void;
    auto deleteTable(const std::string &tableName) -> void;
    auto addColumn(const std::string &tableName, const Column &column) -> void;
    auto removeColumn(const std::string &tableName, const std::string &columnName) -> void;

    // Operacje DML
    auto insertInto(const std::string &tableName, const std::string &columnName, Row inputRow) -> void;
    auto update(const std::string &tableName, const std::string &columnName,
                Row newValue, const std::string &whereClause) -> void;
    auto deleteDataFromColumn(const std::string &tableName, const std::string &columnName,
                              const Row& dataToDelete) -> void;

    // Operacje DQL
    auto select(const std::string &tableName, const std::vector<std::string> &columns,
                const std::string &whereClause) -> std::vector<Row>;

    auto getTables() const -> const std::vector<Table>;
    auto addTable(const Table &table) -> void;
    auto matchCondition(Row &row, const std::unique_ptr<Expression> &expression) -> bool;
    auto evaluateExpression(Row &row, const std::unique_ptr<Expression> &expression) -> bool;
    auto getColumnType(const std::string &tableName, const std::string &columnName) const -> std::string;
    auto isNumeric(const std::string &str) -> bool;



private:
    std::vector<Table> tables;






};


#endif //DATABASE2_DATABASE_H

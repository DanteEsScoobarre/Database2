//
// Created by rozsh on 16/01/2024.
//

#ifndef DATABASE2_DATABASE_H
#define DATABASE2_DATABASE_H
#pragma once
#include "Prerequestion.h"
#include "Expression.h"
#include "Parser.h"


// Represents a column in a table
struct Column {
    std::string name;
    std::string type;
};

// Represents a row in a table
struct Row {
    std::vector<std::string> Data;

    auto getValue(const std::string& columnName, const std::vector<Column>& columns) -> std::string const;

};


// Represents a table in the database
struct Table {
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;
};

class Database {
public:
    Database() = default;

    auto validateDataType(const std::string &value, const std::string &type) -> bool;

    // DDL Operations
    auto createTable(const std::string &tableName, const std::vector<Column> &columns) -> void;

    auto deleteTable(const std::string &tableName) -> void;

    auto addColumn(const std::string &tableName, const Column &column) -> void;

    auto removeColumn(const std::string &tableName, const std::string &columnName) -> void;

    // DML Operations
    auto insertInto(const std::string &tableName, const Row &row) -> void;

    auto update(const std::string &tableName, const Row &newRow, const std::string &whereClause) -> void;

    auto deleteFrom(const std::string &tableName, const std::string &whereClause) -> void;

    // DQL Operations
    auto select(const std::string &tableName, const std::vector<std::string> &columns,
                const std::string &whereClause) -> std::vector<Row>;

    //Saving to a file
    auto getTables() const -> const std::vector<Table>;

    auto addTable(const Table &table) -> void;

    auto clear() -> void;
    auto matchCondition(const Row &row, const std::string &condition) -> bool;

    auto evaluateExpression(const Row &row, const Expression &expression) -> bool ;
private:
    std::vector<Table> tables;



};


#endif //DATABASE2_DATABASE_H

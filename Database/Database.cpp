#include "Database.h"
#include "Row.h"


auto findTable(std::vector<Table> &tables, const std::string &tableName) {
    return std::ranges::find_if(tables.begin(), tables.end(), [&tableName](const Table &table) {
        return table.name == tableName;
    });
}

auto Database::createTable(const std::string &tableName, const std::vector<Column> &columns) -> void {
    if (findTable(tables, tableName) != tables.end()) {
        throw std::runtime_error("Table already exists.");
    }

    tables.push_back({tableName, columns, {}});
}

auto Database::deleteTable(const std::string &tableName) -> void {
    auto it = findTable(tables, tableName);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found.");
    }

    tables.erase(it);
}

auto Database::addColumn(const std::string &tableName, const Column &column) -> void {
    auto it = findTable(tables, tableName);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found.");
    }

    it->columns.push_back(column);
}

auto Database::removeColumn(const std::string &tableName, const std::string &columnName) -> void {
    auto it = findTable(tables, tableName);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found.");
    }

    auto colIt = std::ranges::find_if(it->columns.begin(), it->columns.end(), [&columnName](const Column &column) {
        return column.name == columnName;
    });

    if (colIt == it->columns.end()) {
        throw std::runtime_error("Column not found.");
    }

    it->columns.erase(colIt);
}


auto Database::insertInto(const std::string &tableName, const std::string &columnName, Row inputRow) -> void {
    auto tableIt = findTable(tables, tableName);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found: " + tableName);
    }


    size_t columnIndex = 0;
    bool columnFound = false;
    for (; columnIndex < tableIt->columns.size(); ++columnIndex) {
        if (tableIt->columns[columnIndex].name == columnName) {
            columnFound = true;
            break;
        }
    }

    if (!columnFound) {
        throw std::runtime_error("Column not found: " + columnName);
    }

    if (inputRow.Data.size() != 1) {
        throw std::runtime_error("Input row should have exactly one value for the specified column");
    }

    std::string columnType = getColumnType(tableIt->name, columnName);
    std::string &data = inputRow.Data[0];
    if ((columnType == "int" && !isInteger(data)) ||
        (columnType == "string" && !isString(data)) ||
        (columnType == "bool" && !isBoolean(data))) {
        throw std::runtime_error("Data type mismatch for column: " + columnName);
    }
    bool rowUpdated = false;
    for (auto &row: tableIt->rows) {
        if (row.canUpdate(columnIndex)) {
            row.setValue(columnIndex, inputRow);
            rowUpdated = true;
            break;
        }
    }

    if (!rowUpdated) {
        Row newRow(tableIt->columns);
        newRow.setValue(columnIndex, inputRow);
        tableIt->rows.push_back(newRow);
        std::cout << "Data inserting into columns in: " + tableName << std::endl;
    }
}


auto Database::update(const std::string &tableName, const std::string &columnName,
                      Row newValue, const std::string &whereClause) -> void {
    auto tableIt = findTable(tables, tableName);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found.");
    }

    auto columnIt = std::ranges::find_if(tableIt->columns.begin(), tableIt->columns.end(),
                                         [&columnName](const Column &col) {
                                             return col.name == columnName;
                                         });
    if (columnIt == tableIt->columns.end()) {
        throw std::runtime_error("Column not found.");
    }
    std::size_t columnIndex = std::distance(tableIt->columns.begin(), columnIt);


    if (newValue.Data.size() <= columnIndex) {
        throw std::runtime_error("New value does not contain data for the specified column.");
    }
    const std::string &updatedValue = newValue.Data[columnIndex];


    Parser parser;
    std::unique_ptr<Expression> whereExpression;
    if (!whereClause.empty()) {
        whereExpression = parser.parseWhereClause(whereClause);
    }

    for (auto &row: tableIt->rows) {
        if (row.Data.size() > columnIndex) {

            if (whereClause.empty() || (whereExpression && evaluateExpression(row, whereExpression))) {
                row.Data[columnIndex] = updatedValue;
            }
        }
    }
}


auto Database::deleteDataFromColumn(const std::string &tableName, const std::string &columnName,
                                    const Row &dataToDelete) -> void {
    auto tableIt = findTable(tables, tableName);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found.");
    }

    auto columnIt = std::ranges::find_if(tableIt->columns.begin(), tableIt->columns.end(),
                                         [&columnName](const Column &col) {
                                             return col.name == columnName;
                                         });
    if (columnIt == tableIt->columns.end()) {
        throw std::runtime_error("Column not found.");
    }
    std::size_t columnIndex = std::distance(tableIt->columns.begin(), columnIt);

    if (dataToDelete.Data.size() <= columnIndex) {
        throw std::runtime_error("Data to delete does not contain data for the specified column.");
    }
    const std::string &dataToDeleteValue = dataToDelete.Data[columnIndex];


    std::string columnType = getColumnType(tableIt->name, columnName);


    if ((columnType == "int" && !isInteger(dataToDeleteValue)) ||
        (columnType == "string" && !isString(dataToDeleteValue)) ||
        (columnType == "bool" && !isBoolean(dataToDeleteValue))) {
        throw std::runtime_error("Data type mismatch for column: " + columnName);
    }

    for (auto &row: tableIt->rows) {
        if (row.Data.size() > columnIndex && row.Data[columnIndex] == dataToDeleteValue) {
            row.Data[columnIndex] = "";
        }
    }
}


auto Database::select(const std::string &tableName, const std::vector<std::string> &columns,
                      const std::string &whereClause) -> std::vector<Row> {
    auto tableIt = findTable(tables, tableName);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found.");
    }
    std::vector<Row> result;
    Parser parser;

    std::unique_ptr<Expression> whereExpression;
    if (!whereClause.empty()) {
        whereExpression = parser.parseWhereClause(whereClause);
    }

    for (Row &row: tableIt->rows) {
        if (whereClause.empty() || (whereExpression && evaluateExpression(row, whereExpression))) {
            Row selectedRow(tableIt->columns);
            for (const auto &colName: columns) {
                selectedRow.Data.push_back(row.getValue(colName));
            }
            result.push_back(selectedRow);
        }
    }
    return result;
}


auto Database::getTables() const -> const std::vector<Table> {
    return tables;
}

auto Database::addTable(const Table &table) -> void {
    tables.push_back(table);
}


auto Database::matchCondition(Row &row, const std::unique_ptr<Expression> &expression) -> bool {
    if (!expression) {
        throw std::runtime_error("Expression is null");
    }


    if (expression->logicalOperator == "AND") {
        return matchCondition(row, expression->left) && matchCondition(row, expression->right);
    } else if (expression->logicalOperator == "OR") {
        return matchCondition(row, expression->left) || matchCondition(row, expression->right);
    }

    std::string columnValue = row.getValue(expression->column);


    if (expression->operators == "=") {
        return columnValue == expression->value;
    } else if (expression->operators == "!=") {
        return columnValue != expression->value;
    } else if (expression->operators == ">") {
        return columnValue > expression->value;
    } else if (expression->operators == ">=") {
        return columnValue >= expression->value;
    } else if (expression->operators == "<=") {
        return columnValue <= expression->value;
    }


    throw std::runtime_error("Invalid expression operator");
}


auto Database::evaluateExpression(Row &row, const std::unique_ptr<Expression> &expression) -> bool {
    if (!expression) {
        return true;
    }

    std::string columnValue = row.getValue(expression->column);


    bool isColumnValueNumeric = isNumeric(columnValue);
    bool isExpressionValueNumeric = isNumeric(expression->value);

    if (expression->operators == "=") {
        return columnValue == expression->value;
    } else if (expression->operators == "!=") {
        return columnValue != expression->value;
    } else if (isColumnValueNumeric && isExpressionValueNumeric) {
        int numColumnValue = std::stoi(columnValue);
        int numExpressionValue = std::stoi(expression->value);

        if (expression->operators == ">") {
            return numColumnValue > numExpressionValue;
        } else if (expression->operators == ">=") {
            return numColumnValue >= numExpressionValue;
        } else if (expression->operators == "<") {
            return numColumnValue < numExpressionValue;
        } else if (expression->operators == "<=") {
            return numColumnValue <= numExpressionValue;
        }
    }

    if (expression->logicalOperator == "AND") {
        return evaluateExpression(row, expression->left) && evaluateExpression(row, expression->right);
    } else if (expression->logicalOperator == "OR") {
        return evaluateExpression(row, expression->right) || evaluateExpression(row, expression->left);
    }

    throw std::runtime_error("Unknown or unhandled expression operator");
}

bool Database::isNumeric(const std::string &str) {
    return !str.empty() && std::ranges::find_if(str.begin(),
                                                str.end(), [](unsigned char c) {
                return !std::isdigit(c) && c != '.' && c != '-';
            }) == str.end();
}


auto Row::getValue(const std::string &columnName) -> std::string const {
    if (columns == nullptr) {
        throw std::runtime_error("ERROR: Nie istniejąca kolumna:  " + columnName);
    }

    for (size_t i = 0; i < columns->size(); ++i) {
        if ((*columns)[i].name == columnName) {
            if (i < Data.size()) {
                return Data[i];

            } else {
                return "Data not faund";
            }
        }
    }
    throw std::runtime_error("Error: Column name '" + columnName + "' not found in Row::getValue");
}


auto Row::canUpdate(size_t columnIndex) -> bool {
    return columnIndex < Data.size() && Data[columnIndex].empty();
}

auto Row::setValue(size_t columnIndex, const Row &inputRow) -> void {
    if (columnIndex >= columns->size()) {
        throw std::runtime_error("Column index out of range");
    }
    if (Data.size() <= columnIndex) {
        Data.resize(columns->size(), "");
    }
    if (!inputRow.Data.empty()) {
        Data[columnIndex] = inputRow.Data[0];
    }
}


auto Database::getColumnType(const std::string &tableName, const std::string &columnName) const -> std::string {
    for (const auto &table: tables) {
        if (table.name == tableName) {
            for (const auto &column: table.columns) {
                if (column.name == columnName) {
                    return column.type;
                }
            }
            throw std::runtime_error("Column not found: " + columnName);
        }
    }
    throw std::runtime_error("Table not found: " + tableName);
}

auto Database::isInteger(const std::string &value) -> bool {
    if (value.empty() || ((!isdigit(value[0])) && (value[0] != '-') && (value[0] != '+'))) return false;

    char *p;
    strtol(value.c_str(), &p, 10);

    return (*p == 0);
}

auto Database::isBoolean(const std::string &value) -> bool {
    return value == "true" || value == "false";
}


auto Database::isString(const std::string &value) -> bool {
    return true;
}









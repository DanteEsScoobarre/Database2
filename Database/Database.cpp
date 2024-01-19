#include "Database.h"


auto findTable(std::vector<Table> &tables, const std::string &tableName) {
    return std::ranges::find_if(tables.begin(), tables.end(), [&tableName](const Table &table) {
        return table.name == tableName;
    });
}

auto Database::createTable(const std::string &tableName, const std::vector<Column> &columns) -> void {
    std::cout << "Creating table '" << tableName << "' with columns:" << std::endl;
    for (const auto &column: columns) {
        std::cout << " - " << column.name << " (" << column.type << ")" << std::endl;
    }

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

    std::cout << "Searching for column '" << columnName << "' (Length: " << columnName.length() << ")" << std::endl;

    auto colIt = std::ranges::find_if(it->columns.begin(), it->columns.end(), [&columnName](const Column &column) {
        std::cout << "Checking against '" << column.name << "' (Length: " << column.name.length() << ")" << std::endl;
        return column.name == columnName;
    });

    if (colIt == it->columns.end()) {
        throw std::runtime_error("Column not found.");
    }

    it->columns.erase(colIt);
}



auto Database::insertInto(const std::string &tableName, const Row &row) -> void {
    Parser parser;
    auto tableIt = findTable(tables, tableName);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found.");
    }

    if (row.Data.size() != tableIt->columns.size()) {
        throw std::runtime_error("Row data does not match table columns.");
    }

    // Validate data types for each column
    for (std::size_t i = 0; i < row.Data.size(); ++i) {
        if (!parser.validateDataType(row.Data[i], tableIt->columns[i].type)) {
            throw std::runtime_error("Data type mismatch for column: " + tableIt->columns[i].name);
        }
    }

    tableIt->rows.push_back(row);
}

auto
Database::update(const std::string &tableName, const std::string &columnName, const std::string &newValue) -> void {
    auto tableIt = findTable(tables, tableName);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found.");
    }

    // Find the index of the column to be updated
    auto columnIt = std::ranges::find_if(tableIt->columns.begin(), tableIt->columns.end(), [&columnName](const Column &col) {
        return col.name == columnName;
    });
    if (columnIt == tableIt->columns.end()) {
        throw std::runtime_error("Column not found.");
    }
    std::size_t columnIndex = std::distance(tableIt->columns.begin(), columnIt);

    // Update the specified column in all rows
    for (auto &row: tableIt->rows) {
        if (row.Data.size() > columnIndex) {
            row.Data[columnIndex] = newValue; // Update the data at the specified column index
        }
    }
}

auto Database::deleteDataFromColumn(const std::string &tableName, const std::string &columnName,
                                    const std::string &dataToDelete) -> void {
    auto tableIt = findTable(tables, tableName);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found.");
    }

    auto columnIt = std::ranges::find_if(tableIt->columns.begin(), tableIt->columns.end(), [&columnName](const Column &col) {
        return col.name == columnName;
    });
    if (columnIt == tableIt->columns.end()) {
        throw std::runtime_error("Column not found.");
    }
    std::size_t columnIndex = std::distance(tableIt->columns.begin(), columnIt);

    for (auto &row: tableIt->rows) {
        if (row.Data.size() > columnIndex && row.Data[columnIndex] == dataToDelete) {
            row.Data[columnIndex] = ""; // Or use some null value representation
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

    // Parse the whereClause into an Expression object
    std::unique_ptr<Expression> whereExpression;
    if (!whereClause.empty()) {
        whereExpression = parser.parseWhereClause(whereClause);
    }

    for (Row &row: tableIt->rows) {
        // Check the condition only if whereExpression is not null
        if (whereClause.empty() || (whereExpression && evaluateExpression(row, whereExpression))) {
            Row selectedRow(tableIt->columns); // Use the correct constructor
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

    // Handling logical operators
    if (expression->logicalOperator == "AND") {
        return matchCondition(row, expression->left) && matchCondition(row, expression->right);
    } else if (expression->logicalOperator == "OR") {
        return matchCondition(row, expression->left) || matchCondition(row, expression->right);
    }

    // Assuming 'columns' is a member of 'Row' or can be accessed here
    std::string columnValue = row.getValue(expression->column);

    // Handle comparison operators
    if (expression->operators == "=") {
        return columnValue == expression->value;
    } else if (expression->operators == "!=") {
        return columnValue != expression->value;
    } else if (expression->operators == ">") {
        return columnValue > expression->value; // Note: this is a simplistic comparison
    } else if (expression->operators == ">=") {
        return columnValue >= expression->value; // Note: this is a simplistic comparison
    } else if (expression->operators == "<=") {
        return columnValue <= expression->value; // Note: this is a simplistic comparison
    }

    // If no valid operator found
    throw std::runtime_error("Invalid expression operator");
}


auto Database::evaluateExpression(Row &row, const std::unique_ptr<Expression> &expression) -> bool {
    if (!expression) {
        // Base case: end of a branch in the expression tree
        return true;
    }// Get the value for the specified column from the row
    std::string columnValue = row.getValue(expression->column);

    // Evaluate the comparison
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
    } else if (expression->logicalOperator == "AND") {
        return evaluateExpression(row, expression->left) && evaluateExpression(row, expression->right);
    } else if (expression->logicalOperator == "OR") {
        return evaluateExpression(row, expression->left) || evaluateExpression(row, expression->right);
    }

    // If the expression is not recognized or not handled
    throw std::runtime_error("Unknown or unhandled expression operator");
}



auto Row::getValue(const std::string &columnName) -> std::string const {
    if (columns == nullptr) {
        throw std::runtime_error("Columns pointer is null");
    }

    for (size_t i = 0; i < columns->size(); ++i) {
        if ((*columns)[i].name == columnName && i < Data.size()) {
            return Data[i];
        }
    }

    throw std::runtime_error("Column name not found");
}








#include "Database.h"


auto findTable(std::vector<Table> &tables, const std::string &tableName) {
    return std::find_if(tables.begin(), tables.end(), [&tableName](const Table &table) {
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

    auto colIt = std::find_if(it->columns.begin(), it->columns.end(), [&columnName](const Column &column) {
        return column.name == columnName;
    });

    if (colIt == it->columns.end()) {
        throw std::runtime_error("Column not found.");
    }

    it->columns.erase(colIt);
}

auto Database::validateDataType(const std::string &value, const std::string &type) -> bool {
    if (type == "int") {
        // Check if the entire string is a valid integer
        return std::all_of(value.begin(), value.end(), [](char c) {
            return std::isdigit(c) || c == '-';
        }) && (value.size() > 1 ? value[1] != '-' : true);
    } else if (type == "string") {
        return true;
    } else if (type == "bool") {
        // Check if the string is either "true" or "false"
        return value == "true" || value == "false";
    }
    std::cerr << "Unknown data type: " << type << std::endl;
    return false;
}

auto Database::insertInto(const std::string &tableName, const Row &row) -> void {
    auto tableIt = findTable(tables, tableName);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found.");
    }

    if (row.Data.size() != tableIt->columns.size()) {
        throw std::runtime_error("Row data does not match table columns.");
    }

    // Validate data types for each column
    for (std::size_t i = 0; i < row.Data.size(); ++i) {
        if (!validateDataType(row.Data[i], tableIt->columns[i].type)) {
            throw std::runtime_error("Data type mismatch for column: " + tableIt->columns[i].name);
        }
    }

    tableIt->rows.push_back(row);
}

auto Database::update(const std::string &tableName, const Row &newRow, const std::string &whereClause) -> void {
    auto tableIt = findTable(tables, tableName);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found.");
    }

    // Parse the whereClause (assuming format "column=value")
    auto delimiterPos = whereClause.find('=');
    if (delimiterPos == std::string::npos) {
        throw std::runtime_error("Invalid where clause.");
    }
    std::string columnName = whereClause.substr(0, delimiterPos);
    std::string value = whereClause.substr(delimiterPos + 1);

    // Find the index of the column
    auto columnIt = std::find_if(tableIt->columns.begin(), tableIt->columns.end(), [&columnName](const Column &col) {
        return col.name == columnName;
    });
    if (columnIt == tableIt->columns.end()) {
        throw std::runtime_error("Column not found.");
    }
    std::size_t columnIndex = std::distance(tableIt->columns.begin(), columnIt);

    // Update rows
    for (auto &row: tableIt->rows) {
        if (row.Data.size() > columnIndex && row.Data[columnIndex] == value) {
            row = newRow; // Update the entire row
        }
    }
}

auto Database::deleteFrom(const std::string &tableName, const std::string &whereClause) -> void {
    auto tableIt = findTable(tables, tableName);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found.");
    }

    // Parse the whereClause (assuming format "column=value")
    auto delimiterPos = whereClause.find('=');
    if (delimiterPos == std::string::npos) {
        throw std::runtime_error("Invalid where clause.");
    }
    std::string columnName = whereClause.substr(0, delimiterPos);
    std::string value = whereClause.substr(delimiterPos + 1);
    auto columnIt = std::find_if(tableIt->columns.begin(), tableIt->columns.end(),
                                 [&columnName](const Column &col) {
                                     return col.name == columnName;
                                 });
    if (columnIt == tableIt->columns.end()) {
        throw std::runtime_error("Column not found.");
    }
    std::size_t columnIndex = std::distance(tableIt->columns.begin(), columnIt);

    auto newEnd = std::remove_if(tableIt->rows.begin(), tableIt->rows.end(),
                                 [columnIndex, &value](const Row &row) {
                                     return row.Data.size() > columnIndex && row.Data[columnIndex] == value;
                                 });
    tableIt->rows.erase(newEnd, tableIt->rows.end());
}

auto Database::select(const std::string &tableName, const std::vector<std::string> &columns,
                      const std::string &whereClause) -> std::vector<Row> {
    auto tableIt = findTable(tables, tableName);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found.");
    }
    std::vector<Row> result;
    Parser parser;
    std::unique_ptr<Expression> whereExpression = parser.parseWhereClause(whereClause);

    for (Row &row : tableIt->rows) {
        if (whereClause.empty() || evaluateExpression(row, whereExpression)) {
            // If there's no whereClause or the row matches the expression, add it to the result
            Row selectedRow(tableIt->columns); // Use the correct constructor
            for (const auto& colName : columns) {
                // Fetch the value of a column
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

auto Database::clear() -> void {
    tables.clear();
}

bool Database::matchCondition(Row &row, const std::unique_ptr<Expression> &expression) {
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
    }

    if (expression->logicalOperator == "AND") {
        return evaluateExpression(row, expression->left) && evaluateExpression(row, expression->right);
    } else if (expression->logicalOperator == "OR") {
        return evaluateExpression(row, expression->left) || evaluateExpression(row, expression->right);
    }

    // Get the value for the specified column from the row
    std::string columnValue = row.getValue(expression->column);

    // Evaluate the comparison
    if (expression->operators == "=") {
        return columnValue == expression->value;
    } else if (expression->operators == "!=") {
        return columnValue != expression->value;
    } else if (expression->operators == ">") {
        // Additional logic may be needed for proper numerical comparison
        return columnValue > expression->value;
    } // ... and other operators

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

void Row::setColumns(const std::vector<Column> &cols) {
    columns = &cols;
}




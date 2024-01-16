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
    // Basic validation logic (e.g., check if value is an integer for 'int' type)
    // This is a simple placeholder; real-world applications would require more robust validation
    if (type == "int") {
        return std::all_of(value.begin(), value.end(), [&value](char c) {
            return std::isdigit(c) || (c == '-' && std::isdigit(*(value.begin())));
        });
    }
    // Add more type validations as needed

    return true;
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
    Expression whereExpression = parser.parseWhereClause(whereClause);
    for (Row row : tableIt->rows) {
        if (whereClause.empty() || evaluateExpression(row, whereExpression)) {
            // If there's no whereClause or the row matches the expression, add it to the result
            Row selectedRow;
            for (const auto& colName : columns) {
                // Assuming Row has a method getValue to fetch the value of a column
                selectedRow.Data.push_back(row.getValue(colName, (const std::vector<Column> &) columns));
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

auto Database::matchCondition(const Row& row, const std::string& condition) -> bool {
    // Implement logic to check if a row satisfies the condition
    // Example: Parse 'column=value' and check against row data
    return true; // Placeholder return
}
auto Database::evaluateExpression(const Row& row, const Expression& expression) -> bool{
// Evaluate the expression based on the row data
// This function needs to be implemented to handle the expression tree
// and compare it against the row's values
    return true; // Placeholder for actual evaluation logic
}
std::string const Row::getValue(const std::string& columnName, const std::vector<Column>& columns) {
    Row row;
    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].name == columnName && i < row.Data.size()) {
            return row.Data[i];
        }
    }
    throw std::runtime_error("Column name not found");
};




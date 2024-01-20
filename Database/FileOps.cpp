//
// Created by rozsh on 16/01/2024.
//

#include "FileOps.h"

auto FileOps::saveDatabase(const Database &db, const std::string &filename) -> void {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for writing: " + filename);
    }

    file << "{\n"; // Start of the database JSON object
    for (const auto &table : db.getTables()) {
        file << "  \"TABLE\": \"" << table.name << "\",\n";
        file << "  \"COLUMNS\": [\n";
        for (size_t i = 0; i < table.columns.size(); ++i) {
            file << "    {\"name\": \"" << table.columns[i].name << "\", \"type\": \"" << table.columns[i].type << "\"}";
            if (i < table.columns.size() - 1) {
                file << ",";
            }
            file << "\n";
        }
        file << "  ],\n";
        file << "  \"ROWS\": [\n";
        for (size_t j = 0; j < table.rows.size(); ++j) {
            file << "    {";
            for (size_t k = 0; k < table.rows[j].Data.size(); ++k) {
                file << "\"" << table.columns[k].name << "\": \"" << table.rows[j].Data[k] << "\"";
                if (k < table.rows[j].Data.size() - 1) {
                    file << ", ";
                }
            }
            file << "}";
            if (j < table.rows.size() - 1) {
                file << ",";
            }
            file << "\n";
        }
        file << "  ]\n";
        if (&table != &db.getTables().back()) {
            file << "},\n";
        }
    }
    file << "}\n"; // End of the database JSON object
    file.close();
}

auto FileOps::trim(const std::string &str) -> std::string {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, (last - first + 1));
}


auto FileOps::loadDatabase(const std::string &filename) -> Database {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for reading: " + filename);
    }

    Database db;
    std::string line;
    Table currentTable;
    bool inTable = false;

    while (getline(file, line)) {
        line = trim(line);

        if (line.starts_with("\"TABLE\":")) {
            std::string tableName = extractValue(line, "\"TABLE\":");
            std::cout << "Found a table start: " << tableName << std::endl;
            if (!tableName.empty()) {
                currentTable.name = tableName;
                currentTable.columns.clear();
                currentTable.rows.clear();
                inTable = true;
            }
        }
        else if (inTable && line.starts_with("\"COLUMNS\":")) {
            std::cout << "Processing COLUMNS" << std::endl;
            while (getline(file, line) && !line.starts_with("  ],")) {
                line = trim(line);
                std::cout << "Column line: " << line << std::endl;

                if (line.starts_with("{")) {
                    std::string columnName = extractValue(line, "\"name\":");
                    std::string columnType = extractValue(line, "\"type\":");

                    if (!columnName.empty() && !columnType.empty()) {
                        Column column;
                        column.name = columnName;
                        column.type = columnType;
                        currentTable.columns.push_back(column);
                        std::cout << "Added column: " << columnName << " of type " << columnType << std::endl;
                    } else {
                        std::cerr << "Failed to parse column from line: " << line << std::endl;
                    }
                }
            }
        }
        else if (inTable && line.starts_with("\"ROWS\":")) {
            std::cout << "Processing ROWS" << std::endl;
            std::vector<Row> tempRows; // Temporary rows for accumulation

            int currentRowIndex = 0; // Index to track which row is being populated
            while (getline(file, line)) {
                line = trim(line);
                if (line.starts_with("{")) {
                    std::istringstream iss(line);
                    std::string token;
                    getline(iss, token, ':'); // Extract column name
                    token = trim(token);
                    size_t nameStart = token.find('\"') + 1;
                    size_t nameEnd = token.find('\"', nameStart);
                    std::string columnName = token.substr(nameStart, nameEnd - nameStart);

                    getline(iss, token, '}'); // Extract column value
                    std::string value = trim(extractValue(token, "\"\":"));

                    // Find the corresponding column index
                    auto it = std::ranges::find_if(currentTable.columns.begin(), currentTable.columns.end(),
                                           [&](const Column& col) { return col.name == columnName; });
                    if (it != currentTable.columns.end()) {
                        int colIndex = std::distance(currentTable.columns.begin(), it);
                        if (currentRowIndex < tempRows.size()) {
                            tempRows[currentRowIndex].Data[colIndex] = value;
                        }
                    }
                } else if (line == "],") {
                    // Break out of the loop once the end of rows is reached
                    break;
                } else if (line == "},") {
                    currentRowIndex++; // Move to the next row
                }
            }

            // Add the accumulated rows to the currentTable
            for (auto& row : tempRows) {
                if (!row.Data.empty()) {
                    currentTable.rows.push_back(row);
                    std::cout << "Added row" << std::endl; // Debug output for each added row
                }
            }
        }

        else if (inTable && line == "},") {
            db.addTable(currentTable);
            inTable = false;
        }
    }

    file.close();
    return db;
}


auto FileOps::extractValue(const std::string &line, const std::string &key) -> std::string {
    size_t keyPos = line.find(key);
    if (keyPos == std::string::npos) {
        return "";
    }

    size_t start = line.find('\"', keyPos + key.length()) + 1;
    size_t end = line.find('\"', start);
    if (start == std::string::npos || end == std::string::npos || start >= end) {
        return "";
    }

    return line.substr(start, end - start);
}
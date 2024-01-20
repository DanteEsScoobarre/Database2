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
        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "\"TABLE\":") {
            // Start a new table
            inTable = true;
            std::string tableName;
            getline(iss, tableName, ',');
            tableName = tableName.substr(2, tableName.length() - 3); // Remove quotes and trailing comma
            currentTable.name = tableName;
            currentTable.columns.clear();
            currentTable.rows.clear();
        } else if (key == "\"COLUMNS\":") {
            // Process columns
            while (getline(file, line) && line.find("]") == std::string::npos) {
                Column column;
                std::string colName, colType;
                std::istringstream colStream(line);
                colStream >> key; // This should be "{" or "}"
                if (key == "{") {
                    colStream >> key; // "name":
                    getline(colStream, colName, ',');
                    colName = colName.substr(2, colName.length() - 3); // Remove quotes and leading "name": "
                    colStream >> key; // "type":
                    getline(colStream, colType, '}');
                    colType = colType.substr(2, colType.length() - 4); // Remove quotes and leading "type": "
                    column.name = colName;
                    column.type = colType;
                    currentTable.columns.push_back(column);
                }
            }
        } else if (key == "\"ROWS\":") {
            // Process rows
            while (getline(file, line) && line.find("]") == std::string::npos) {
                if (line.find("{") != std::string::npos) {
                    Row row(currentTable.columns);
                    std::string value;
                    for (auto& column : currentTable.columns) {
                        getline(file, line, ':');
                        getline(file, value, ',');
                        value = value.substr(2, value.length() - 3); // Remove quotes and trailing comma
                        row.Data.push_back(value);
                    }
                    getline(file, line); // Finish the current row line
                    currentTable.rows.push_back(row);
                }
            }
        } else if (key == "},") {
            // End of a table
            db.addTable(currentTable);
            inTable = false;
        }
    }

    file.close();
    return db;
}


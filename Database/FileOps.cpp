//
// Created by rozsh on 16/01/2024.
//

#include "FileOps.h"

auto FileOps::saveDatabase(const Database &db, const std::string &filename) -> void {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for writing: " + filename);
    }

    for (const auto &table: db.getTables()) {
        file << "TABLE " << table.name << "\n";
        for (const auto &column: table.columns) {
            file << "COLUMN " << column.name << " " << column.type << "\n";
        }
        for (const auto &row: table.rows) {
            file << "ROW";
            for (const auto &value: row.Data) {
                file << " " << value;
            }
            file << "\n";
        }
        file << "ENDTABLE\n";
    }

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
    bool isTableOpen = false;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "TABLE") {
            isTableOpen = true;
            iss >> currentTable.name;
            currentTable.columns.clear();
            currentTable.rows.clear();
        } else if (token == "COLUMN" && isTableOpen) {
            Column column;
            iss >> column.name >> column.type;
            currentTable.columns.push_back(column);
        } else if (token == "ROW" && isTableOpen) {
            Row row;
            std::string value;
            while (iss >> value) {
                row.Data.push_back(value);
            }
            currentTable.rows.push_back(row);
        } else if (token == "ENDTABLE") {
            db.addTable(currentTable);
            isTableOpen = false;
        }
    }
    file.close();
    return db;
}



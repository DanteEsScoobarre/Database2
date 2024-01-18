#include "CLI.h"


void CLI::run() {
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "quit" || input == "exit") {
            break;
        }


        try {
            Command command = parser.parseSQLCommand(input);
            executeCommand(command);
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}


void CLI::executeCommand(const Command &command) {
    try {
        if (command.type == "CREATE") {
            db.createTable(command.tableName, command.columns);
        } else if (command.type == "DROP") {
            db.deleteTable(command.tableName);
        } else if (command.type == "ADD") {
            // Assuming ADD command adds a column to a table
            for (const auto &column: command.columns) {
                db.addColumn(command.tableName, column);
            }
        } else if (command.type == "INSERT") {
            // Assuming INSERT command inserts a new row
            db.insertInto(command.tableName, Row(command.data)); // Row construction might differ
        } else if (command.type == "UPDATE") {
            if (command.updatedData.Data.empty()) {
                throw std::runtime_error("No data provided for update");
            }
            // Use the first value from the updatedData's Data vector for the update
            std::string newValue = command.updatedData.Data[0];
            db.update(command.tableName, command.columnName, newValue);
        } else if (command.type == "DELETE") {
            // Assuming DELETE command deletes from a table
            db.deleteDataFromColumn(command.tableName, command.columnName, command.value);
        } else if (command.type == "REMOVE") {
            db.removeColumn(command.tableName, command.columnName);
        } else if (command.type == "SELECT") {
            std::vector<std::string> columnNames;
            for (const auto &column: command.columns) {
                columnNames.push_back(column.name); // Extracting the name from each Column
            }

            auto rows = db.select(command.tableName, columnNames, command.whereClause);
            displaySelectedRows(rows, columnNames);
        } else if (command.type == "SAVE") {
            handleSaveCommand(command.value);
        } else if (command.type == "LOAD") {
            handleLoadCommand(command.value);
        } else {
            throw std::runtime_error("Invalid command");
        }

    } catch (const std::exception &e) {
        std::cerr << "Database operation error: " << e.what() << std::endl;
    }
}

auto CLI::displaySelectedRows(const std::vector<Row> &rows, const std::vector<std::string> &columnNames) -> void {
    for (Row row: rows) {
        for (const auto &columnName: columnNames) {
            std::cout << row.getValue(columnName) << " ";
        }
        std::cout << std::endl;
    }
}

void CLI::handleSaveCommand(const std::string &filename) {
    try {
        FileOps::saveDatabase(db, filename);
        std::cout << "Database saved successfully to " << filename << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error saving database: " << e.what() << std::endl;
    }
}

void CLI::handleLoadCommand(const std::string &filename) {
    try {
        db = FileOps::loadDatabase(filename);
        std::cout << "Database loaded successfully from " << filename << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error loading database: " << e.what() << std::endl;
    }
}



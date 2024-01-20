#include "CLI.h"


auto CLI::run() -> void {
    std::string input;
    while (true) {
        std::cout << ">> ";
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


auto CLI::executeCommand(const Command &command) -> void {
    FileOps fileops;
    try {
        if (command.type == "CREATE") {
            db.createTable(command.tableName, command.columns);
        } else if (command.type == "DROP") {
            db.deleteTable(command.tableName);
        } else if (command.type == "ADD") {
            for (const auto &column: command.columns) {
                db.addColumn(command.tableName, column);
            }
        } else if (command.type == "INSERT") {
            // Assuming INSERT command inserts a new row
            db.insertInto(command.tableName, command.columnName,Row(command.data));
        } else if (command.type == "UPDATE") {
            if (command.updatedData.Data.empty()) {
                throw std::runtime_error("No data provided for update");
            }
            std::string newValue = command.updatedData.Data[0];
            db.update(command.tableName, command.columnName, newValue);
        } else if (command.type == "DELETE") {

            db.deleteDataFromColumn(command.tableName, command.columnName, command.dataToDelete);
        } else if (command.type == "REMOVE") {
            db.removeColumn(command.tableName, command.columnName);
        } else if (command.type == "SELECT") {
            std::vector<std::string> columnNames;
            for (const auto &column: command.columns) {
                columnNames.push_back(column.name);
            }

            auto rows = db.select(command.tableName, columnNames, command.whereClause);
            displaySelectedRows(rows);
        } else if (command.type == "SAVE") {
            fileops.saveDatabase(db, command.value);
        }
        else if (command.type == "LOAD") {
            fileops.loadDatabase(command.value);
        }
        else {
            throw std::runtime_error("Invalid command");
        }

    } catch (const std::exception &e) {
        std::cerr << "Database operation error: " << e.what() << std::endl;
    }
}

auto CLI::displaySelectedRows(const std::vector<Row> &rows) -> void {
    for (const Row& row : rows) {
        for (const auto& value : row.Data) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
}






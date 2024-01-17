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
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}




void CLI::executeCommand(const Command& command) {
    try {
        if (command.type == "CREATE") {
            db.createTable(command.tableName, command.columns);
        } else if (command.type == "DROP") {
            db.deleteTable(command.tableName);
        } else if (command.type == "ADD") {
            // Assuming ADD command adds a column to a table
            for (const auto& column : command.columns) {
                db.addColumn(command.tableName, column);
            }
        } else if (command.type == "INSERT") {
            // Assuming INSERT command inserts a new row
            db.insertInto(command.tableName, Row(command.data)); // Row construction might differ
        } else if (command.type == "UPDATE") {
            // Assuming UPDATE command updates a row
            db.update(command.tableName, Row(command.updatedData), command.whereClause);
        } else if (command.type == "DELETE") {
            // Assuming DELETE command deletes from a table
            db.deleteFrom(command.tableName, command.whereClause);
        }
        // ... handle other command types ...
    } catch (const std::exception& e) {
        std::cerr << "Database operation error: " << e.what() << std::endl;
    }
}


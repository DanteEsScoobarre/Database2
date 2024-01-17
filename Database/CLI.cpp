#include "CLI.h"


CLI::CLI(Database &db) : db(db) {
}

void CLI::run() {
    std::string input;
    while (true) {
        std::cout << "Enter command: ";
        std::getline(std::cin, input);

        if (input == "exit" || input == "quit") {
            break;
        }

        try {
            processCommand(input);
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

void CLI::processCommand(const std::string &input) {
    std::istringstream iss(input);
    std::string command;
    iss >> command;

    if (command == "CREATE" || command == "DROP" || command == "ADD") {
        // Handle DDL commands
        handleDDL(iss, command);
    } else if (command == "INSERT" || command == "UPDATE" || command == "DELETE") {
        // Handle DML commands
        handleDML(iss, command);
    } else if (command == "SELECT") {
        // Handle DQL commands
        handleDQL(iss);
    } else {
        std::cout << "Unknown command." << std::endl;
    }
}

void CLI::handleDDL(std::istringstream &iss, const std::string &command) {
    std::string temp;
    if (command == "CREATE") {
        std::string tableName, columnDefinition;
        std::vector<Column> columns;
        iss >> tableName; // Get the table name

        // Read the rest of the line after the table name
        std::string restOfLine;
        std::getline(iss, restOfLine);
        std::istringstream columnStream(restOfLine);

        while (std::getline(columnStream, columnDefinition, '}')) {
            size_t start = columnDefinition.find('{');
            if (start == std::string::npos) {
                std::cout << "Expected '{' before column definition.\n";
                break;
            }
            std::string columnDetails = columnDefinition.substr(start + 1);

            std::istringstream detailsStream(columnDetails);
            std::string columnName, columnType;
            if (!(detailsStream >> columnName >> columnType)) {
                std::cout << "Invalid column definition.\n";
                break;
            }

            columns.push_back(Column{columnName, columnType});
        }

        try {
            if (!columns.empty()) {
                db.createTable(tableName, columns);
                std::cout << "Table created successfully.\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error creating table: " << e.what() << std::endl;
        }
        std::cout << "Current tables in the database:" << std::endl;
        for (const auto& table : db.getTables()) {
            std::cout << table.name << std::endl;
        }
    } else if (command == "DROP") {
        std::string tableName;
        iss >> temp; // Skip "TABLE"
        iss >> tableName;
        db.deleteTable(tableName);
        std::cout << "Table dropped successfully.\n";
    }    else if (command == "ADD") {
        std::string columnDefinition, tableName, columnName, columnType;

        // Read the column definition until 'INTO'
        std::getline(iss, columnDefinition, 'I');  // Reads up to 'I' of 'INTO'
        size_t start = columnDefinition.find('{');
        size_t end = columnDefinition.rfind('}');
        if (start == std::string::npos || end == std::string::npos) {
            std::cout << "Expected '{' and '}' around column definition.\n";
            return;
        }
        std::string columnDetails = columnDefinition.substr(start + 1, end - start - 1);
        std::istringstream detailsStream(columnDetails);
        if (!(detailsStream >> columnName >> columnType)) {
            std::cout << "Invalid column definition.\n";
            return;
        }

        // Read the table name
        iss >> temp;
        iss >> tableName;

        // Add column to the table
        try {
            db.addColumn(tableName, Column{columnName, columnType});
            std::cout << "Column added successfully.\n";
        } catch (const std::exception& e) {
            std::cerr << "Error adding column: " << e.what() << std::endl;
        }
    } else {
        std::cout << "Invalid DDL command.\n";
    }
}


void CLI::handleDML(std::istringstream &iss, const std::string &command) {
    if (command == "INSERT") {
        std::string tableName, temp;
        std::vector<std::string> values;
        iss >> temp; // Skip "INTO"
        iss >> tableName >> temp; // Skip "VALUES"
        while (iss >> temp) {
            if (temp == "(" || temp == ",") continue;
            if (temp == ")") break;
            values.push_back(temp);
        }

        Row newRow; // Create a new Row object
        newRow.Data = values; // Assign the values
        db.insertInto(tableName, newRow);
        std::cout << "Data inserted successfully.\n";
    } else if (command == "UPDATE") {
        std::string tableName, temp, setClause, whereClause;
        Row newRow;

        iss >> tableName; // Get the table name

        // Process SET clause
        iss >> temp; // Expecting the "SET" keyword
        if (temp != "SET") {
            std::cout << "Expected 'SET' keyword.\n";
            return;
        }

        // Extracting the SET clause until the WHERE keyword or end of input
        while (iss >> temp && temp != "WHERE") {
            if (temp == ",") continue; // Skip commas
            std::string columnName = temp;
            iss >> temp; // Skip '='
            std::string value;
            iss >> value;
            newRow.Data.push_back(value); // Assuming the order of values matches the columns
        }

        // Check if WHERE clause is present
        if (temp == "WHERE") {
            std::getline(iss, whereClause); // Get the rest of the line as the WHERE clause
        }

        try {
            db.update(tableName, newRow, whereClause);
            std::cout << "Data updated successfully.\n";
        } catch (const std::exception &e) {
            std::cerr << "Error executing UPDATE command: " << e.what() << std::endl;
        }
    } else if (command == "DELETE") {
        std::string tableName, temp, whereClause;

        iss >> temp; // Expecting the "FROM" keyword
        if (temp != "FROM") {
            std::cout << "Expected 'FROM' keyword.\n";
            return;
        }

        iss >> tableName; // Get the table name

        // Check if WHERE clause is present
        iss >> temp; // Expecting the "WHERE" keyword
        if (temp != "WHERE") {
            std::cout << "Expected 'WHERE' keyword.\n";
            return;
        }

        std::getline(iss, whereClause); // Get the rest of the line as the WHERE clause

        try {
            db.deleteFrom(tableName, whereClause);
            std::cout << "Data deleted successfully.\n";
        } catch (const std::exception &e) {
            std::cerr << "Error executing DELETE command: " << e.what() << std::endl;
        }
    } else {
        std::cout << "Invalid DML command.\n";
    }
}


void CLI::handleDQL(std::istringstream &iss) {
    // Example: SELECT column1, column2 FROM tableName WHERE condition
    std::string temp, tableName, whereClause;
    std::vector<std::string> columns;
    bool selectAll = false;

    // Parse columns or '*' for all columns
    while (iss >> temp && temp != "FROM") {
        if (temp == ",") continue;
        if (temp == "*") {
            selectAll = true;
            break;
        }
        columns.push_back(temp);
    }

    if (!selectAll && columns.empty()) {
        std::cout << "No columns specified for selection.\n";
        return;
    }

    // Parse table name
    if (!(iss >> tableName)) {
        std::cout << "Table name not specified.\n";
        return;
    }

    // Check if there is a WHERE clause
    if (iss >> temp && temp == "WHERE") {
        std::getline(iss, whereClause);
    }

    try {
        std::vector<Row> rows = db.select(tableName, columns, whereClause);
        displaySelectedRows(rows, columns, selectAll);
    } catch (const std::exception &e) {
        std::cerr << "Error executing SELECT command: " << e.what() << std::endl;
    }
}

void CLI::displaySelectedRows(const std::vector<Row> &rows, const std::vector<std::string> &columns, bool selectAll) {
    for (Row row: rows) {
        if (selectAll) {
            // Display all columns
            for (size_t i = 0; i < row.columns->size(); ++i) {
                std::cout << row.getValue((*row.columns)[i].name) << "\t";
            }
        } else {
            // Display specified columns
            for (const auto &columnName: columns) {
                std::cout << row.getValue(columnName) << "\t";
            }
        }
        std::cout << std::endl;
    }
}

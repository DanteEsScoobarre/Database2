#ifndef CLI_H
#define CLI_H
#pragma once
#include "Database.h"
#include "FileOps.h"
#include <SFML/Graphics.hpp>

class CLI {
public:
    CLI(Database& db);
    void run();

private:
    Database& db;
    sf::RenderWindow window;
    // Additional members for handling input and output
    void processCommand(const std::string &input);

    void handleDDL(std::istringstream &iss, const std::string &command);

    void handleDQL(std::istringstream &iss);

    void handleDML(std::istringstream &iss, const std::string &command);

    void displaySelectedRows(const std::vector<Row> &rows, const std::vector<std::string> &columns, bool selectAll);
};

#endif // CLI_H

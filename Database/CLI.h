#ifndef CLI_H
#define CLI_H
#pragma once
#include "Database.h"
#include "Parser.h"
#include "FileOps.h"
#include <SFML/Graphics.hpp>

class CLI {
public:
    CLI(Database& Database, Parser& parser) : db(Database), parser(parser) {
        this->db;
        this->parser;
    }
    auto displaySelectedRows(const std::vector<Row>& rows, const std::vector<std::string>& columnNames) -> void;
    void executeCommand(const Command &command);
    void run();

private:
    Database& db;
    Parser& parser;
    sf::RenderWindow window;


};

#endif // CLI_H

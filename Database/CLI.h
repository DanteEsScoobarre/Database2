#ifndef CLI_H
#define CLI_H
#pragma once
#include "Database.h"
#include "Parser.h"
#include "FileOps.h"

class CLI {
public:
    CLI(Database& Database, Parser& parser) : db(Database), parser(parser) {
    }
    auto displaySelectedRows(const std::vector<Row> &rows) -> void;
    auto executeCommand(const Command &command) -> void;
    auto run() -> void;


private:
    Database& db;
    Parser& parser;
};

#endif // CLI_H

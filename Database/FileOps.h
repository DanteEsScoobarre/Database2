#ifndef FILEOPS_H
#define FILEOPS_H
#pragma once
#include "Database.h"


class FileOps {
public:
    auto saveDatabase(const Database &db, const std::string &filename) -> void;

    auto loadDatabase(const std::string &filename) -> Database;

    auto  trim(const std::string &str) -> std::string;

    auto extractValue(const std::string &line, const std::string &key) -> std::string;
};

#endif // FILEOPS_H





#ifndef FILEOPS_H
#define FILEOPS_H
#pragma once
#include "Database.h"


class FileOps {
public:
    static auto saveDatabase(const Database &db, const std::string &filename) -> void;

    static auto loadDatabase(const std::string &filename) -> Database;
};

#endif // FILEOPS_H





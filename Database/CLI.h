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
};

#endif // CLI_H
